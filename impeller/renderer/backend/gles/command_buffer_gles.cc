// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "impeller/renderer/backend/gles/command_buffer_gles.h"

#include "impeller/base/config.h"
#include "impeller/renderer/backend/gles/blit_pass_gles.h"
#include "impeller/renderer/backend/gles/render_pass_gles.h"

namespace impeller {

CommandBufferGLES::CommandBufferGLES(std::weak_ptr<const Context> context,
                                     ReactorGLES::Ref reactor)
    : CommandBuffer(std::move(context)),
      reactor_(std::move(reactor)),
      is_valid_(reactor_ && reactor_->IsValid()) {}

CommandBufferGLES::~CommandBufferGLES() = default;

// |CommandBuffer|
void CommandBufferGLES::SetLabel(const std::string& label) const {
  // Cannot support.
}

// |CommandBuffer|
bool CommandBufferGLES::IsValid() const {
  return is_valid_;
}

// |CommandBuffer|
bool CommandBufferGLES::OnSubmitCommands(SyncMode sync_mode,
                                         CompletionCallback callback) {
  const auto result = reactor_->React();
  if (result) {
    const auto& gl = reactor_->GetProcTable();
    switch (sync_mode) {
      case CommandBuffer::SyncMode::kWaitUntilScheduled:
        gl.Flush();
        break;
      case CommandBuffer::SyncMode::kWaitUntilCompleted:
        gl.Finish();
        break;
      case CommandBuffer::SyncMode::kDontCare:
        break;
    }
  }
  if (callback) {
    callback(result ? CommandBuffer::Status::kCompleted
                    : CommandBuffer::Status::kError);
  }
  return result;
}

// |CommandBuffer|
std::shared_ptr<RenderPass> CommandBufferGLES::OnCreateRenderPass(
    RenderTarget target) {
  if (!IsValid()) {
    return nullptr;
  }
  auto pass = std::shared_ptr<RenderPassGLES>(
      new RenderPassGLES(context_, target, reactor_));
  if (!pass->IsValid()) {
    return nullptr;
  }
  return pass;
}

// |CommandBuffer|
std::shared_ptr<BlitPass> CommandBufferGLES::OnCreateBlitPass() const {
  if (!IsValid()) {
    return nullptr;
  }
  auto pass = std::shared_ptr<BlitPassGLES>(new BlitPassGLES(reactor_));
  if (!pass->IsValid()) {
    return nullptr;
  }
  return pass;
}

// |CommandBuffer|
std::shared_ptr<ComputePass> CommandBufferGLES::OnCreateComputePass() const {
  // Compute passes aren't supported until GLES 3.2, at which point Vulkan is
  // available anyway.
  return nullptr;
}

}  // namespace impeller
