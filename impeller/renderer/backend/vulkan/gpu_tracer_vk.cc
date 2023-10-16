// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "impeller/renderer/backend/vulkan/gpu_tracer_vk.h"

#include <utility>
#include "fml/logging.h"
#include "fml/trace_event.h"
#include "impeller/base/validation.h"
#include "impeller/renderer/backend/vulkan/context_vk.h"
#include "vulkan/vulkan_enums.hpp"

namespace impeller {

static constexpr uint32_t kPoolSize = 64u;

GPUTracerVK::GPUTracerVK(const std::shared_ptr<DeviceHolder>& device_holder)
    : device_holder_(device_holder) {
  timestamp_period_ = device_holder_->GetPhysicalDevice()
                          .getProperties()
                          .limits.timestampPeriod;
  if (timestamp_period_ <= 0) {
    // The device does not support timestamp queries.
    return;
  }
  // Disable tracing in release mode.
#ifdef IMPELLER_DEBUG
  valid_ = true;
#endif
}

void GPUTracerVK::MarkFrameStart() {
  in_frame_ = true;
}

void GPUTracerVK::MarkFrameEnd() {
  if (!valid_) {
    return;
  }
  Lock lock(trace_state_mutex_);
  current_state_ = (current_state_ + 1) % 16;

  auto& state = trace_states_[current_state_];
  FML_DCHECK(state.pending_buffers == 0u);

  state.pending_buffers = 0;
  state.current_index = 0;
  in_frame_ = false;
}

void GPUTracerVK::RecordCmdBufferStart(const vk::CommandBuffer& buffer) {
  if (!valid_ || !in_frame_) {
    return;
  }
  Lock lock(trace_state_mutex_);
  auto& state = trace_states_[current_state_];

  // Initialize the query pool for the first query on each frame.
  if (state.pending_buffers == 0) {
    vk::QueryPoolCreateInfo info;
    info.queryCount = kPoolSize;
    info.queryType = vk::QueryType::eTimestamp;

    auto [status, pool] =
        device_holder_->GetDevice().createQueryPoolUnique(info);
    if (status != vk::Result::eSuccess) {
      VALIDATION_LOG << "Failed to create query pool.";
      return;
    }
    trace_states_[current_state_].query_pool = std::move(pool);
    buffer.resetQueryPool(trace_states_[current_state_].query_pool.get(), 0,
                          kPoolSize);
  }

  // We size the query pool to kPoolSize, but Flutter applications can create an
  // unbounded amount of work per frame. If we encounter this, stop recording
  // cmds.
  if (state.current_index >= kPoolSize) {
    return;
  }

  buffer.writeTimestamp(vk::PipelineStageFlagBits::eTopOfPipe,
                        trace_states_[current_state_].query_pool.get(),
                        state.current_index);
  state.pending_buffers += 1;
  state.current_index += 1;
}

size_t GPUTracerVK::RecordCmdBufferEnd(const vk::CommandBuffer& buffer) {
  if (!valid_ || !in_frame_) {
    return 0;
  }
  Lock lock(trace_state_mutex_);
  GPUTraceState& state = trace_states_[current_state_];

  if (state.current_index >= kPoolSize) {
    return current_state_;
  }

  buffer.writeTimestamp(vk::PipelineStageFlagBits::eBottomOfPipe,
                        state.query_pool.get(), state.current_index);

  state.current_index += 1;
  return current_state_;
}

void GPUTracerVK::OnFenceComplete(size_t frame_index) {
  if (!valid_) {
    return;
  }
  Lock lock(trace_state_mutex_);
  GPUTraceState& state = trace_states_[frame_index];
  if (state.pending_buffers == 0) {
    return;
  }
  state.pending_buffers -= 1;

  if (state.pending_buffers == 0) {
    auto buffer_count = state.current_index;
    std::vector<uint64_t> bits(buffer_count);

    auto result = device_holder_->GetDevice().getQueryPoolResults(
        state.query_pool.get(), 0, state.current_index,
        buffer_count * sizeof(uint64_t), bits.data(), sizeof(uint64_t),
        vk::QueryResultFlagBits::e64);
    // This may return VK_NOT_READY if the query couldn't be completed, or if
    // there are queries still pending. From local testing, this happens
    // occassionally on very expensive frames. Its unclear if we can do anything
    // about this, because by design this should only signal after all cmd
    // buffers have signaled. Adding VK_QUERY_RESULT_WAIT_BIT to the flags
    // passed to getQueryPoolResults seems like it would fix this, but actually
    // seems to result in more stuck query errors. Better to just drop them and
    // move on.
    if (result != vk::Result::eSuccess) {
      return;
    }

    uint64_t smallest_timestamp = std::numeric_limits<uint64_t>::max();
    uint64_t largest_timestamp = 0;
    for (auto i = 0u; i < bits.size(); i++) {
      smallest_timestamp = std::min(smallest_timestamp, bits[i]);
      largest_timestamp = std::max(largest_timestamp, bits[i]);
    }
    auto gpu_ms =
        (((largest_timestamp - smallest_timestamp) * timestamp_period_) /
         1000000);
    FML_TRACE_COUNTER("flutter", "GPUTracer",
                      reinterpret_cast<int64_t>(this),  // Trace Counter ID
                      "FrameTimeMS", gpu_ms);
  }
}

}  // namespace impeller
