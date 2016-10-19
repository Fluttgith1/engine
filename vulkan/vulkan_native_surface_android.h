// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_VULKAN_VULKAN_NATIVE_SURFACE_ANDROID_H_
#define FLUTTER_VULKAN_VULKAN_NATIVE_SURFACE_ANDROID_H_

#include "flutter/vulkan/vulkan_native_surface.h"
#include "lib/ftl/macros.h"

struct ANativeWindow;
typedef struct ANativeWindow ANativeWindow;

namespace vulkan {

class VulkanNativeSurfaceAndroid : public VulkanNativeSurface {
 public:
  VulkanNativeSurfaceAndroid(ANativeWindow* native_window);

  ~VulkanNativeSurfaceAndroid();

  const char* ExtensionName() const override;

  uint32_t SkiaExtensionName() const override;

  VkSurfaceKHR CreateSurfaceHandle(
      VulkanProcTable& vk,
      const VulkanHandle<VkInstance>& instance) const override;

  bool IsValid() const override;

  SkISize GetSize() const override;

 private:
  ANativeWindow* native_window_;

  FTL_DISALLOW_COPY_AND_ASSIGN(VulkanNativeSurfaceAndroid);
};

}  // namespace vulkan

#endif  // FLUTTER_VULKAN_VULKAN_NATIVE_SURFACE_ANDROID_H_
