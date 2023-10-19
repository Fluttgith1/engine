// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include <cstddef>

#include "impeller/base/backend_cast.h"
#include "impeller/core/shader_types.h"
#include "impeller/geometry/size.h"
#include "impeller/renderer/capabilities.h"

namespace impeller {

class ProcTableGLES;

//------------------------------------------------------------------------------
/// @brief      The Vulkan layers and extensions wrangler.
///
class CapabilitiesGLES final
    : public Capabilities,
      public BackendCast<CapabilitiesGLES, Capabilities> {
 public:
  explicit CapabilitiesGLES(const ProcTableGLES& gl);

  CapabilitiesGLES(const CapabilitiesGLES&) = delete;

  CapabilitiesGLES(CapabilitiesGLES&&) = delete;

  CapabilitiesGLES& operator=(const CapabilitiesGLES&) = delete;

  CapabilitiesGLES& operator=(CapabilitiesGLES&&) = delete;

  // Must be at least 8.
  size_t max_combined_texture_image_units = 8;

  // Must be at least 16.
  size_t max_cube_map_texture_size = 16;

  // Must be at least 16.
  size_t max_fragment_uniform_vectors = 16;

  // Must be at least 1.
  size_t max_renderbuffer_size = 1;

  // Must be at least 8.
  size_t max_texture_image_units = 8;

  // Must be at least 64.
  ISize max_texture_size = ISize{64, 64};

  // Must be at least 8.
  size_t max_varying_vectors = 8;

  // Must be at least 8.
  size_t max_vertex_attribs = 8;

  // May be 0.
  size_t max_vertex_texture_image_units = 0;

  // Must be at least 128.
  size_t max_vertex_uniform_vectors = 128;

  // Must be at least display size.
  ISize max_viewport_dims;

  // May be 0.
  size_t num_compressed_texture_formats = 0;

  // May be 0.
  size_t num_shader_binary_formats = 0;

  size_t GetMaxTextureUnits(ShaderStage stage) const;

  // |Capabilities|
  bool SupportsOffscreenMSAA() const override;

  // |Capabilities|
  bool SupportsSSBO() const override;

  // |Capabilities|
  bool SupportsBufferToTextureBlits() const override;

  // |Capabilities|
  bool SupportsTextureToTextureBlits() const override;

  // |Capabilities|
  bool SupportsFramebufferFetch() const override;

  // |Capabilities|
  bool SupportsCompute() const override;

  // |Capabilities|
  bool SupportsComputeSubgroups() const override;

  // |Capabilities|
  bool SupportsReadFromOnscreenTexture() const override;

  // |Capabilities|
  bool SupportsReadFromResolve() const override;

  // |Capabilities|
  bool SupportsDecalSamplerAddressMode() const override;

  // |Capabilities|
  bool SupportsDeviceTransientTextures() const override;

  // |Capabilities|
  bool SupportsNativeAdvancedBlends() const override;

  // |Capabilities|
  PixelFormat GetDefaultColorFormat() const override;

  // |Capabilities|
  PixelFormat GetDefaultStencilFormat() const override;

  // |Capabilities|
  PixelFormat GetDefaultDepthStencilFormat() const override;

 private:
  bool supports_framebuffer_fetch_ = false;
  bool supports_decal_sampler_address_mode_ = false;
};

}  // namespace impeller
