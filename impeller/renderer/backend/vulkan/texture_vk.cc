// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "impeller/renderer/backend/vulkan/texture_vk.h"

#include "impeller/renderer/backend/vulkan/command_buffer_vk.h"
#include "impeller/renderer/backend/vulkan/command_encoder_vk.h"
#include "impeller/renderer/backend/vulkan/formats_vk.h"

namespace impeller {

TextureVK::TextureVK(std::weak_ptr<Context> context,
                     std::shared_ptr<TextureSourceVK> source)
    : Texture(source->GetTextureDescriptor()),
      context_(std::move(context)),
      source_(std::move(source)) {}

TextureVK::~TextureVK() = default;

void TextureVK::SetLabel(std::string_view label) {
  auto context = context_.lock();
  if (!context) {
    // The context may have died.
    return;
  }
  ContextVK::Cast(*context).SetDebugName(GetImage(), label);
  ContextVK::Cast(*context).SetDebugName(GetImageView(), label);
}

bool TextureVK::OnSetContents(const uint8_t* contents,
                              size_t length,
                              IRect region,
                              size_t slice) {
  if (!IsValid() || !contents) {
    return false;
  }

  const auto& desc = GetTextureDescriptor();

  // Out of bounds access.
  if (length != desc.GetByteSizeOfRegion(region)) {
    VALIDATION_LOG << "Illegal to set contents for invalid size: " << length
                   << " vs " << desc.GetByteSizeOfRegion(region);
    return false;
  }

  auto context = context_.lock();
  if (!context) {
    VALIDATION_LOG << "Context died before setting contents on texture.";
    return false;
  }

  auto staging_buffer =
      context->GetResourceAllocator()->CreateBufferWithCopy(contents, length);

  if (!staging_buffer) {
    VALIDATION_LOG << "Could not create staging buffer.";
    return false;
  }

  auto cmd_buffer = context->CreateCommandBuffer();

  if (!cmd_buffer) {
    return false;
  }

  const auto encoder = CommandBufferVK::Cast(*cmd_buffer).GetEncoder();

  if (!encoder->Track(staging_buffer) || !encoder->Track(source_)) {
    return false;
  }

  const auto& vk_cmd_buffer = encoder->GetCommandBuffer();

  BarrierVK barrier;
  barrier.cmd_buffer = vk_cmd_buffer;
  barrier.new_layout = vk::ImageLayout::eTransferDstOptimal;
  barrier.src_access = {};
  barrier.src_stage = vk::PipelineStageFlagBits::eTopOfPipe;
  barrier.dst_access = vk::AccessFlagBits::eTransferWrite;
  barrier.dst_stage = vk::PipelineStageFlagBits::eTransfer;

  if (!SetLayout(barrier)) {
    return false;
  }

  vk::BufferImageCopy copy;
  copy.bufferOffset = 0u;
  copy.bufferRowLength = 0u;    // 0u means tightly packed per spec.
  copy.bufferImageHeight = 0u;  // 0u means tightly packed per spec.
  copy.imageOffset.x = region.GetX();
  copy.imageOffset.y = region.GetY();
  copy.imageOffset.z = 0u;
  copy.imageExtent.width = region.GetWidth();
  copy.imageExtent.height = region.GetHeight();
  copy.imageExtent.depth = 1u;
  copy.imageSubresource.aspectMask =
      ToImageAspectFlags(GetTextureDescriptor().format);
  copy.imageSubresource.mipLevel = 0u;
  copy.imageSubresource.baseArrayLayer = slice;
  copy.imageSubresource.layerCount = 1u;

  vk_cmd_buffer.copyBufferToImage(
      DeviceBufferVK::Cast(*staging_buffer).GetBuffer(),  // src buffer
      GetImage(),                                         // dst image
      barrier.new_layout,                                 // dst image layout
      1u,                                                 // region count
      &copy                                               // regions
  );

  // Transition to shader-read.
  {
    BarrierVK barrier;
    barrier.cmd_buffer = vk_cmd_buffer;
    barrier.src_access = vk::AccessFlagBits::eColorAttachmentWrite |
                         vk::AccessFlagBits::eTransferWrite;
    barrier.src_stage = vk::PipelineStageFlagBits::eColorAttachmentOutput |
                        vk::PipelineStageFlagBits::eTransfer;
    barrier.dst_access = vk::AccessFlagBits::eShaderRead;
    barrier.dst_stage = vk::PipelineStageFlagBits::eFragmentShader;

    barrier.new_layout = vk::ImageLayout::eShaderReadOnlyOptimal;

    if (!SetLayout(barrier)) {
      return false;
    }
  }

  return context->GetCommandQueue()->Submit({cmd_buffer}).ok();
}

bool TextureVK::OnSetContents(const BufferView& buffer_view,
                              IRect region,
                              size_t slice) {
  if (!IsValid() || !buffer_view) {
    return false;
  }

  const auto& desc = GetTextureDescriptor();

  // Out of bounds access.
  // if (length != desc.GetByteSizeOfRegion(region)) {
  //   VALIDATION_LOG << "Illegal to set contents for invalid size: " << length
  //                  << " vs " << desc.GetByteSizeOfRegion(region);
  //   return false;
  // }

  auto context = context_.lock();
  if (!context) {
    VALIDATION_LOG << "Context died before setting contents on texture.";
    return false;
  }

  auto cmd_buffer = context->CreateCommandBuffer();

  if (!cmd_buffer) {
    return false;
  }

  const auto encoder = CommandBufferVK::Cast(*cmd_buffer).GetEncoder();

  if (!encoder->Track(buffer_view.buffer) || !encoder->Track(source_)) {
    return false;
  }

  const auto& vk_cmd_buffer = encoder->GetCommandBuffer();

  BarrierVK barrier;
  barrier.cmd_buffer = vk_cmd_buffer;
  barrier.new_layout = vk::ImageLayout::eTransferDstOptimal;
  barrier.src_access = {};
  barrier.src_stage = vk::PipelineStageFlagBits::eTopOfPipe;
  barrier.dst_access = vk::AccessFlagBits::eTransferWrite;
  barrier.dst_stage = vk::PipelineStageFlagBits::eTransfer;

  if (!SetLayout(barrier)) {
    return false;
  }

  vk::BufferImageCopy copy;
  copy.bufferOffset = buffer_view.range.offset;
  copy.bufferRowLength = 0u;    // 0u means tightly packed per spec.
  copy.bufferImageHeight = 0u;  // 0u means tightly packed per spec.
  copy.imageOffset.x = region.GetX();
  copy.imageOffset.y = region.GetY();
  copy.imageOffset.z = 0u;
  copy.imageExtent.width = region.GetWidth();
  copy.imageExtent.height = region.GetHeight();
  copy.imageExtent.depth = 1u;
  copy.imageSubresource.aspectMask =
      ToImageAspectFlags(GetTextureDescriptor().format);
  copy.imageSubresource.mipLevel = 0u;
  copy.imageSubresource.baseArrayLayer = slice;
  copy.imageSubresource.layerCount = 1u;

  vk_cmd_buffer.copyBufferToImage(
      DeviceBufferVK::Cast(*buffer_view.buffer).GetBuffer(),  // src buffer
      GetImage(),                                             // dst image
      barrier.new_layout,  // dst image layout
      1u,                  // region count
      &copy                // regions
  );

  // Transition to shader-read.
  {
    BarrierVK barrier;
    barrier.cmd_buffer = vk_cmd_buffer;
    barrier.src_access = vk::AccessFlagBits::eColorAttachmentWrite |
                         vk::AccessFlagBits::eTransferWrite;
    barrier.src_stage = vk::PipelineStageFlagBits::eColorAttachmentOutput |
                        vk::PipelineStageFlagBits::eTransfer;
    barrier.dst_access = vk::AccessFlagBits::eShaderRead;
    barrier.dst_stage = vk::PipelineStageFlagBits::eFragmentShader;

    barrier.new_layout = vk::ImageLayout::eShaderReadOnlyOptimal;

    if (!SetLayout(barrier)) {
      return false;
    }
  }

  return context->GetCommandQueue()->Submit({cmd_buffer}).ok();
}

bool TextureVK::OnSetContents(const ContentUpdate updates[],
                              size_t update_count,
                              size_t slice) {
  if (!IsValid()) {
    return false;
  }

  const auto& desc = GetTextureDescriptor();

  auto context = context_.lock();
  if (!context) {
    VALIDATION_LOG << "Context died before setting contents on texture.";
    return false;
  }

  auto cmd_buffer = context->CreateCommandBuffer();

  if (!cmd_buffer) {
    return false;
  }

  const auto encoder = CommandBufferVK::Cast(*cmd_buffer).GetEncoder();

  if (!encoder->Track(source_)) {
    return false;
  }

  const auto& vk_cmd_buffer = encoder->GetCommandBuffer();

  BarrierVK barrier;
  barrier.cmd_buffer = vk_cmd_buffer;
  barrier.new_layout = vk::ImageLayout::eTransferDstOptimal;
  barrier.src_access = {};
  barrier.src_stage = vk::PipelineStageFlagBits::eTopOfPipe;
  barrier.dst_access = vk::AccessFlagBits::eTransferWrite;
  barrier.dst_stage = vk::PipelineStageFlagBits::eTransfer;

  if (!SetLayout(barrier)) {
    return false;
  }

  for (auto i = 0u; i < update_count; i++) {
    const auto& buffer_view = updates[i].buffer_view;
    const auto& region = updates[i].region;

    if (!encoder->Track(buffer_view.buffer)) {
      return false;
    }

    vk::BufferImageCopy copy;
    copy.bufferOffset = buffer_view.range.offset;
    copy.bufferRowLength = 0u;    // 0u means tightly packed per spec.
    copy.bufferImageHeight = 0u;  // 0u means tightly packed per spec.
    copy.imageOffset.x = region.GetX();
    copy.imageOffset.y = region.GetY();
    copy.imageOffset.z = 0u;
    copy.imageExtent.width = region.GetWidth();
    copy.imageExtent.height = region.GetHeight();
    copy.imageExtent.depth = 1u;
    copy.imageSubresource.aspectMask =
        ToImageAspectFlags(GetTextureDescriptor().format);
    copy.imageSubresource.mipLevel = 0u;
    copy.imageSubresource.baseArrayLayer = slice;
    copy.imageSubresource.layerCount = 1u;

    vk_cmd_buffer.copyBufferToImage(
        DeviceBufferVK::Cast(*buffer_view.buffer).GetBuffer(),  // src buffer
        GetImage(),                                             // dst image
        barrier.new_layout,  // dst image layout
        1u,                  // region count
        &copy                // regions
    );
  }

  // Transition to shader-read.
  {
    BarrierVK barrier;
    barrier.cmd_buffer = vk_cmd_buffer;
    barrier.src_access = vk::AccessFlagBits::eTransferWrite;
    barrier.src_stage = vk::PipelineStageFlagBits::eTransfer;
    barrier.dst_access = vk::AccessFlagBits::eShaderRead;
    barrier.dst_stage = vk::PipelineStageFlagBits::eFragmentShader;

    barrier.new_layout = vk::ImageLayout::eShaderReadOnlyOptimal;

    if (!SetLayout(barrier)) {
      return false;
    }
  }

  return context->GetCommandQueue()->Submit({cmd_buffer}).ok();
}

bool TextureVK::IsValid() const {
  return !!source_;
}

ISize TextureVK::GetSize() const {
  return GetTextureDescriptor().size;
}

vk::Image TextureVK::GetImage() const {
  return source_->GetImage();
}

vk::ImageView TextureVK::GetImageView() const {
  return source_->GetImageView();
}

std::shared_ptr<const TextureSourceVK> TextureVK::GetTextureSource() const {
  return source_;
}

bool TextureVK::SetLayout(const BarrierVK& barrier) const {
  return source_ ? source_->SetLayout(barrier).ok() : false;
}

vk::ImageLayout TextureVK::SetLayoutWithoutEncoding(
    vk::ImageLayout layout) const {
  return source_ ? source_->SetLayoutWithoutEncoding(layout)
                 : vk::ImageLayout::eUndefined;
}

vk::ImageLayout TextureVK::GetLayout() const {
  return source_ ? source_->GetLayout() : vk::ImageLayout::eUndefined;
}

vk::ImageView TextureVK::GetRenderTargetView() const {
  return source_->GetRenderTargetView();
}

void TextureVK::SetFramebuffer(
    const SharedHandleVK<vk::Framebuffer>& framebuffer) {
  framebuffer_ = framebuffer;
}

void TextureVK::SetRenderPass(
    const SharedHandleVK<vk::RenderPass>& render_pass) {
  render_pass_ = render_pass;
}

SharedHandleVK<vk::Framebuffer> TextureVK::GetFramebuffer() const {
  return framebuffer_;
}

SharedHandleVK<vk::RenderPass> TextureVK::GetRenderPass() const {
  return render_pass_;
}

}  // namespace impeller
