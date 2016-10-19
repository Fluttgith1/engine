// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_ANDROID_ANDROID_SURFACE_VULKAN_H_
#define FLUTTER_SHELL_PLATFORM_ANDROID_ANDROID_SURFACE_VULKAN_H_

#include <memory>
#include "flutter/shell/platform/android/android_native_window.h"
#include "flutter/shell/platform/android/android_surface.h"
#include "flutter/vulkan/vulkan_window.h"
#include "lib/ftl/macros.h"

namespace shell {

class AndroidSurfaceVulkan : public AndroidSurface {
 public:
  AndroidSurfaceVulkan(ftl::RefPtr<AndroidNativeWindow> native_window);

  ~AndroidSurfaceVulkan();

  bool IsValid() const override;

  std::unique_ptr<Surface> CreateGPUSurface() override;

  SkISize OnScreenSurfaceSize() const override;

  bool OnScreenSurfaceResize(const SkISize& size) const override;

  bool ResourceContextMakeCurrent() override;

 private:
  bool valid_;
  ftl::RefPtr<AndroidNativeWindow> native_window_;

  FTL_DISALLOW_COPY_AND_ASSIGN(AndroidSurfaceVulkan);
};

}  // namespace shell

#endif  // FLUTTER_SHELL_PLATFORM_ANDROID_ANDROID_SURFACE_VULKAN_H_
