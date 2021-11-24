// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/embedder/embedder_surface_vulkan.h"

#include "flutter/shell/common/shell_io_manager.h"
#include "include/gpu/GrDirectContext.h"
#include "include/gpu/vk/GrVkBackendContext.h"
#include "include/gpu/vk/GrVkExtensions.h"
#include "shell/gpu/gpu_surface_vulkan.h"
#include "shell/gpu/gpu_surface_vulkan_delegate.h"

namespace flutter {

EmbedderSurfaceVulkan::EmbedderSurfaceVulkan(
    VkInstance instance,
    VkPhysicalDevice physical_device,
    VkDevice device,
    uint32_t queue_family_index,
    VkQueue queue,
    VulkanDispatchTable vulkan_dispatch_table,
    std::shared_ptr<EmbedderExternalViewEmbedder> external_view_embedder)
    : vk_(vulkan_dispatch_table.get_instance_proc_address),
      device_(vk_,
              vulkan::VulkanHandle<VkPhysicalDevice>{physical_device},
              vulkan::VulkanHandle<VkDevice>{device},
              queue_family_index,
              vulkan::VulkanHandle<VkQueue>{queue}),
      vulkan_dispatch_table_(vulkan_dispatch_table),
      external_view_embedder_(external_view_embedder) {
  // Make sure all required members of the dispatch table are checked.
  if (!vulkan_dispatch_table_.get_instance_proc_address ||
      !vulkan_dispatch_table_.get_next_image ||
      !vulkan_dispatch_table_.present_image) {
    return;
  }

  vk_.SetupInstanceProcAddresses(vulkan::VulkanHandle<VkInstance>{instance});
  vk_.SetupDeviceProcAddresses(vulkan::VulkanHandle<VkDevice>{device});
  if (!vk_.IsValid()) {
    FML_LOG(ERROR) << "VulkanProcTable invalid.";
    return;
  }

  main_context_ = CreateGrContext(instance, ContextType::kRender);
  // TODO(bdero): Add a second (optional) queue+family index to the Embedder API
  //             to allow embedders to specify a dedicated transfer queue for
  //             use by the resource context. Queue families with graphics
  //             capability can always be used for memory transferring, but it
  //             would be adventageous to use a dedicated transter queue here.
  resource_context_ = CreateGrContext(instance, ContextType::kResource);

  valid_ = main_context_ && resource_context_;
}

EmbedderSurfaceVulkan::~EmbedderSurfaceVulkan() = default;

// |GPUSurfaceVulkanDelegate|
const vulkan::VulkanProcTable& EmbedderSurfaceVulkan::vk() {
  return vk_;
}

// |GPUSurfaceVulkanDelegate|
VkImage EmbedderSurfaceVulkan::AcquireImage(const SkISize& size) {
  return vulkan_dispatch_table_.get_next_image(size);
}

// |GPUSurfaceVulkanDelegate|
bool EmbedderSurfaceVulkan::PresentImage(VkImage image) {
  return vulkan_dispatch_table_.present_image(image);
}

// |EmbedderSurface|
bool EmbedderSurfaceVulkan::IsValid() const {
  return valid_;
}

// |EmbedderSurface|
std::unique_ptr<Surface> EmbedderSurfaceVulkan::CreateGPUSurface() {
  const bool render_to_surface = !external_view_embedder_;
  return std::make_unique<GPUSurfaceVulkan>(this, main_context_,
                                            render_to_surface);
}

// |EmbedderSurface|
sk_sp<GrDirectContext> EmbedderSurfaceVulkan::CreateResourceContext() const {
  return resource_context_;
}

sk_sp<GrDirectContext> EmbedderSurfaceVulkan::CreateGrContext(
    VkInstance instance,
    ContextType context_type) const {
  uint32_t skia_features = 0;
  if (!device_.GetPhysicalDeviceFeaturesSkia(&skia_features)) {
    FML_LOG(ERROR) << "Failed to get physical device features.";

    return nullptr;
  }

  auto get_proc = vk_.CreateSkiaGetProc();
  if (get_proc == nullptr) {
    FML_LOG(ERROR) << "Failed to create Vulkan getProc for Skia.";
    return nullptr;
  }

  GrVkExtensions extensions;

  GrVkBackendContext backend_context = {};
  backend_context.fInstance = instance;
  backend_context.fPhysicalDevice = device_.GetPhysicalDeviceHandle();
  backend_context.fDevice = device_.GetHandle();
  backend_context.fQueue = device_.GetQueueHandle();
  backend_context.fGraphicsQueueIndex = device_.GetGraphicsQueueIndex();
  backend_context.fMinAPIVersion = VK_MAKE_VERSION(1, 0, 0);
  backend_context.fMaxAPIVersion = VK_MAKE_VERSION(1, 0, 0);
  backend_context.fFeatures = skia_features;
  backend_context.fVkExtensions = &extensions;
  backend_context.fGetProc = get_proc;
  backend_context.fOwnsInstanceAndDevice = false;

  // TODO(bdero): Activate MEMORY_REQUIREMENTS_2 if available because VMA (the
  //              allocator used by Skia) knows how to take advantage of these
  //              features.
  /*
    const char* device_extensions[] = {
        VK_KHR_GET_MEMORY_REQUIREMENTS_2_EXTENSION_NAME,
    };
    extensions.init(backend_context.fGetProc, backend_context.fInstance,
                       backend_context.fPhysicalDevice, 0, nullptr,
                       countof(device_extensions), device_extensions);
  */

  GrContextOptions options =
      MakeDefaultContextOptions(context_type, GrBackendApi::kVulkan);
  options.fReduceOpsTaskSplitting = GrContextOptions::Enable::kNo;
  return GrDirectContext::MakeVulkan(backend_context, options);
}

}  // namespace flutter
