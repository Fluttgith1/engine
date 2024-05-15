// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_IMPELLER_RENDERER_BACKEND_METAL_BLIT_PASS_MTL_H_
#define FLUTTER_IMPELLER_RENDERER_BACKEND_METAL_BLIT_PASS_MTL_H_

#include <Metal/Metal.h>

#include "impeller/renderer/blit_pass.h"

namespace impeller {

class BlitPassMTL final : public BlitPass {
 public:
  // |RenderPass|
  ~BlitPassMTL() override;

 private:
  friend class CommandBufferMTL;

  id<MTLBlitCommandEncoder> encoder_ = nil;
  id<MTLCommandBuffer> buffer_ = nil;
  bool is_valid_ = false;
  bool is_metal_trace_active_ = false;
  // Many parts of the codebase will start writing to a render pass but
  // never submit them. This boolean is used to track if a submit happened
  // so that in the dtor we can always ensure the render pass is finished.
  mutable bool did_finish_encoding_ = false;

  explicit BlitPassMTL(id<MTLCommandBuffer> buffer);

  // |BlitPass|
  bool IsValid() const override;

  // |BlitPass|
  void OnSetLabel(std::string label) override;

  // |BlitPass|
  bool EncodeCommands(
      const std::shared_ptr<Allocator>& transients_allocator) const override;

  // |BlitPass|
  bool OnCopyTextureToTextureCommand(std::shared_ptr<Texture> source,
                                     std::shared_ptr<Texture> destination,
                                     IRect source_region,
                                     IPoint destination_origin,
                                     std::string label) override;

  // |BlitPass|
  bool OnCopyTextureToBufferCommand(std::shared_ptr<Texture> source,
                                    std::shared_ptr<DeviceBuffer> destination,
                                    IRect source_region,
                                    size_t destination_offset,
                                    std::string label) override;
  // |BlitPass|
  bool OnCopyBufferToTextureCommand(BufferView source,
                                    std::shared_ptr<Texture> destination,
                                    IRect destination_region,
                                    std::string label,
                                    uint32_t slice) override;

  // |BlitPass|
  bool OnGenerateMipmapCommand(std::shared_ptr<Texture> texture,
                               std::string label) override;

  BlitPassMTL(const BlitPassMTL&) = delete;

  BlitPassMTL& operator=(const BlitPassMTL&) = delete;
};

}  // namespace impeller

#endif  // FLUTTER_IMPELLER_RENDERER_BACKEND_METAL_BLIT_PASS_MTL_H_
