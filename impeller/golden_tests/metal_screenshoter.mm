// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/impeller/golden_tests/metal_screenshoter.h"

#include <CoreImage/CoreImage.h>
#include "impeller/renderer/backend/metal/texture_mtl.h"
#define GLFW_INCLUDE_NONE
#include "third_party/glfw/include/GLFW/glfw3.h"

namespace impeller {
namespace testing {

MetalScreenshoter::MetalScreenshoter() {
  FML_CHECK(::glfwInit() == GLFW_TRUE);
  playground_ = PlaygroundImpl::Create(PlaygroundBackend::kMetal);
  aiks_context_.reset(new AiksContext(playground_->GetContext()));
}

std::unique_ptr<MetalScreenshot> MetalScreenshoter::MakeScreenshot(
    Picture&& picture,
    const ISize& size) {
  std::shared_ptr<Image> image = picture.ToImage(*aiks_context_, size);
  std::shared_ptr<Texture> texture = image->GetTexture();
  id<MTLTexture> metal_texture =
      std::static_pointer_cast<TextureMTL>(texture)->GetMTLTexture();

  if (metal_texture.pixelFormat != MTLPixelFormatBGRA8Unorm) {
    return {};
  }

  CIImage* ciImage = [[CIImage alloc] initWithMTLTexture:metal_texture
                                                 options:@{}];
  if (!ciImage) {
    return {};
  }

  CIContext* context = [CIContext contextWithOptions:nil];

  CGImageRef cgImage = [context createCGImage:ciImage
                                     fromRect:[ciImage extent]];

  return std::unique_ptr<MetalScreenshot>(new MetalScreenshot(cgImage));
}

}  // namespace testing
}  // namespace impeller
