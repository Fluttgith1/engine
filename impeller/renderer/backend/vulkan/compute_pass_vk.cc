// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "impeller/renderer/backend/vulkan/compute_pass_vk.h"

#include "impeller/renderer/backend/vulkan/barrier_vk.h"
#include "impeller/renderer/backend/vulkan/command_buffer_vk.h"
#include "impeller/renderer/backend/vulkan/compute_pipeline_vk.h"
#include "impeller/renderer/backend/vulkan/device_buffer_vk.h"
#include "impeller/renderer/backend/vulkan/formats_vk.h"
#include "impeller/renderer/backend/vulkan/sampler_vk.h"
#include "impeller/renderer/backend/vulkan/texture_vk.h"
#include "vulkan/vulkan_enums.hpp"
#include "vulkan/vulkan_structs.hpp"

namespace impeller {

ComputePassVK::ComputePassVK(std::shared_ptr<const Context> context,
                             std::shared_ptr<CommandBufferVK> command_buffer)
    : ComputePass(std::move(context)),
      command_buffer_(std::move(command_buffer)) {
  // TOOD(dnfield): This should be moved to caps. But for now keeping this
  // in parallel with Metal.
  max_wg_size_ = ContextVK::Cast(GetContext())
                     .GetPhysicalDevice()
                     .getProperties()
                     .limits.maxComputeWorkGroupSize;
  is_valid_ = true;
}

ComputePassVK::~ComputePassVK() = default;

bool ComputePassVK::IsValid() const {
  return is_valid_;
}

void ComputePassVK::OnSetLabel(const std::string& label) {
  if (label.empty()) {
    return;
  }
  label_ = label;
}

// |RenderPass|
void ComputePassVK::SetCommandLabel(std::string_view label) {
#ifdef IMPELLER_DEBUG
  command_buffer_->GetEncoder()->PushDebugGroup(label);
  has_label_ = true;
#endif  // IMPELLER_DEBUG
}

// |ComputePass|
void ComputePassVK::SetPipeline(
    const std::shared_ptr<Pipeline<ComputePipelineDescriptor>>& pipeline) {
  const auto& pipeline_vk = ComputePipelineVK::Cast(*pipeline);
  const vk::CommandBuffer& command_buffer_vk =
      command_buffer_->GetEncoder()->GetCommandBuffer();
  command_buffer_vk.bindPipeline(vk::PipelineBindPoint::eCompute,
                                 pipeline_vk.GetPipeline());
  pipeline_layout_ = pipeline_vk.GetPipelineLayout();

  auto descriptor_result =
      command_buffer_->GetEncoder()->AllocateDescriptorSets(
          pipeline_vk.GetDescriptorSetLayout(), ContextVK::Cast(*context_));
  if (!descriptor_result.ok()) {
    return;
  }
  descriptor_set_ = descriptor_result.value();
  pipeline_valid_ = true;
}

// |ComputePass|
fml::Status ComputePassVK::Compute(const ISize& grid_size) {
  if (grid_size.IsEmpty() || !pipeline_valid_) {
    return fml::Status(fml::StatusCode::kCancelled,
                       "Invalid pipeline or empty grid.");
  }

  const ContextVK& context_vk = ContextVK::Cast(*context_);
  for (auto i = 0u; i < descriptor_write_offset_; i++) {
    write_workspace_[i].dstSet = descriptor_set_;
  }

  context_vk.GetDevice().updateDescriptorSets(descriptor_write_offset_,
                                              write_workspace_.data(), 0u, {});
  const vk::CommandBuffer& command_buffer_vk =
      command_buffer_->GetEncoder()->GetCommandBuffer();

  command_buffer_vk.bindDescriptorSets(
      vk::PipelineBindPoint::eCompute,  // bind point
      pipeline_layout_,                 // layout
      0,                                // first set
      1,                                // set count
      &descriptor_set_,                 // sets
      0,                                // offset count
      nullptr                           // offsets
  );

  int64_t width = grid_size.width;
  int64_t height = grid_size.height;

  // Special case for linear processing.
  if (height == 1) {
    int64_t minimum = 1;
    int64_t threadGroups = std::max(
        static_cast<int64_t>(std::ceil(width * 1.0 / max_wg_size_[0] * 1.0)),
        minimum);
    command_buffer_vk.dispatch(threadGroups, 1, 1);
  } else {
    while (width > max_wg_size_[0]) {
      width = std::max(static_cast<int64_t>(1), width / 2);
    }
    while (height > max_wg_size_[1]) {
      height = std::max(static_cast<int64_t>(1), height / 2);
    }
    command_buffer_vk.dispatch(width, height, 1);
  }

#ifdef IMPELLER_DEBUG
  if (has_label_) {
    command_buffer_->GetEncoder()->PopDebugGroup();
  }
  has_label_ = false;
#endif  // IMPELLER_DEBUG

  size_t bound_image_offset_ = 0u;
  size_t bound_buffer_offset_ = 0u;
  size_t descriptor_write_offset_ = 0u;
  bool has_label_ = false;
  bool pipeline_valid_ = false;

  return fml::Status();
}

// |ResourceBinder|
bool ComputePassVK::BindResource(ShaderStage stage,
                                 DescriptorType type,
                                 const ShaderUniformSlot& slot,
                                 const ShaderMetadata& metadata,
                                 BufferView view) {
  return BindResource(slot.binding, type, view);
}

// |ResourceBinder|
bool ComputePassVK::BindResource(ShaderStage stage,
                                 DescriptorType type,
                                 const SampledImageSlot& slot,
                                 const ShaderMetadata& metadata,
                                 std::shared_ptr<const Texture> texture,
                                 std::shared_ptr<const Sampler> sampler) {
  if (bound_buffer_offset_ >= kMaxBindings) {
    return false;
  }
  const TextureVK& texture_vk = TextureVK::Cast(*texture);
  const SamplerVK& sampler_vk = SamplerVK::Cast(*sampler);

  if (!command_buffer_->GetEncoder()->Track(texture) ||
      !command_buffer_->GetEncoder()->Track(sampler_vk.GetSharedSampler())) {
    return false;
  }

  vk::DescriptorImageInfo image_info;
  image_info.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
  image_info.sampler = sampler_vk.GetSampler();
  image_info.imageView = texture_vk.GetImageView();
  image_workspace_[bound_image_offset_++] = image_info;

  vk::WriteDescriptorSet write_set;
  write_set.dstBinding = slot.binding;
  write_set.descriptorCount = 1u;
  write_set.descriptorType = ToVKDescriptorType(type);
  write_set.pImageInfo = &image_workspace_[bound_image_offset_ - 1];

  write_workspace_[descriptor_write_offset_++] = write_set;
  return true;
}

bool ComputePassVK::BindResource(size_t binding,
                                 DescriptorType type,
                                 const BufferView& view) {
  if (bound_buffer_offset_ >= kMaxBindings) {
    return false;
  }

  const std::shared_ptr<const DeviceBuffer>& device_buffer = view.buffer;
  auto buffer = DeviceBufferVK::Cast(*device_buffer).GetBuffer();
  if (!buffer) {
    return false;
  }

  if (!command_buffer_->GetEncoder()->Track(device_buffer)) {
    return false;
  }

  uint32_t offset = view.range.offset;

  vk::DescriptorBufferInfo buffer_info;
  buffer_info.buffer = buffer;
  buffer_info.offset = offset;
  buffer_info.range = view.range.length;
  buffer_workspace_[bound_buffer_offset_++] = buffer_info;

  vk::WriteDescriptorSet write_set;
  write_set.dstBinding = binding;
  write_set.descriptorCount = 1u;
  write_set.descriptorType = ToVKDescriptorType(type);
  write_set.pBufferInfo = &buffer_workspace_[bound_buffer_offset_ - 1];

  write_workspace_[descriptor_write_offset_++] = write_set;
  return true;
}

// Note:
// https://github.com/KhronosGroup/Vulkan-Docs/wiki/Synchronization-Examples
// Seems to suggest that anything more finely grained than a global memory
// barrier is likely to be ignored. Confirming this on mobile devices will
// require some experimentation.

// |ComputePass|
void ComputePassVK::AddBufferMemoryBarrier(const BufferView& view) {
  vk::MemoryBarrier barrier;
  barrier.srcAccessMask = vk::AccessFlagBits::eShaderWrite;
  barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;

  command_buffer_->GetEncoder()->GetCommandBuffer().pipelineBarrier(
      vk::PipelineStageFlagBits::eComputeShader,
      vk::PipelineStageFlagBits::eComputeShader, {}, 1, &barrier, 0, {}, 0, {});
}

// |ComputePass|
void ComputePassVK::AddTextureMemoryBarrier(
    const std::shared_ptr<const Texture>& texture) {
  vk::MemoryBarrier barrier;
  barrier.srcAccessMask = vk::AccessFlagBits::eShaderWrite;
  barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;

  command_buffer_->GetEncoder()->GetCommandBuffer().pipelineBarrier(
      vk::PipelineStageFlagBits::eComputeShader,
      vk::PipelineStageFlagBits::eComputeShader, {}, 1, &barrier, 0, {}, 0, {});
}

// |ComputePass|
bool ComputePassVK::EncodeCommands() const {
  // Since we only use global memory barrier, we don't have to worry about
  // compute to compute dependencies across cmd buffers. Instead, we pessimize
  // here and assume that we wrote to a storage image or buffer and that a
  // render pass will read from it. if there are ever scenarios where we end up
  // with compute to compute dependencies this should be revisited.

  // This does not currently handle image barriers as we do not use them
  // for anything.
  vk::MemoryBarrier barrier;
  barrier.srcAccessMask = vk::AccessFlagBits::eShaderWrite;
  barrier.dstAccessMask =
      vk::AccessFlagBits::eIndexRead | vk::AccessFlagBits::eVertexAttributeRead;

  command_buffer_->GetEncoder()->GetCommandBuffer().pipelineBarrier(
      vk::PipelineStageFlagBits::eComputeShader,
      vk::PipelineStageFlagBits::eVertexInput, {}, 1, &barrier, 0, {}, 0, {});

  return true;
}

}  // namespace impeller
