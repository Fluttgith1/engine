// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <cassert>
#include <optional>

#include "flutter/fml/logging.h"
#include "flutter/shell/common/context_options.h"
#include "flutter/testing/test_vulkan_context.h"

#include "flutter/fml/memory/ref_ptr.h"
#include "flutter/fml/native_library.h"
#include "third_party/skia/include/core/SkSurface.h"
#include "third_party/skia/include/gpu/GrDirectContext.h"
#include "third_party/skia/include/gpu/vk/GrVkExtensions.h"
#include "vulkan/vulkan_core.h"

#ifdef FML_OS_MACOSX
#define VULKAN_SO_PATH "libvk_swiftshader.dylib"
#elif FML_OS_WIN
#define VULKAN_SO_PATH "vk_swiftshader.dll"
#else
#define VULKAN_SO_PATH "libvk_swiftshader.so"
#endif

namespace flutter {
namespace testing {

TestVulkanContext::TestVulkanContext() {
  // ---------------------------------------------------------------------------
  // Initialize basic Vulkan state using the Swiftshader ICD.
  // ---------------------------------------------------------------------------

  const char* vulkan_icd = VULKAN_SO_PATH;

  // TODO(96949): Clean this up and pass a native library directly to
  //              VulkanProcTable.
  if (!fml::NativeLibrary::Create(VULKAN_SO_PATH)) {
    FML_LOG(WARNING) << "Couldn't find Vulkan ICD \"" << vulkan_icd
                     << "\", trying \"libvulkan.so\" instead.";
    vulkan_icd = "libvulkan.so";
  }

  vk_ = fml::MakeRefCounted<vulkan::VulkanProcTable>(vulkan_icd);
  if (!vk_ || !vk_->HasAcquiredMandatoryProcAddresses()) {
    FML_LOG(ERROR) << "Proc table has not acquired mandatory proc addresses.";
    return;
  }

  application_ =
      std::unique_ptr<vulkan::VulkanApplication>(new vulkan::VulkanApplication(
          *vk_, "Flutter Unittests", {}, VK_MAKE_VERSION(1, 0, 0),
          VK_MAKE_VERSION(1, 0, 0), true));
  if (!application_->IsValid()) {
    FML_LOG(ERROR) << "Failed to initialize basic Vulkan state.";
    return;
  }
  if (!vk_->AreInstanceProcsSetup()) {
    FML_LOG(ERROR) << "Failed to acquire full proc table.";
    return;
  }

  device_ = application_->AcquireFirstCompatibleLogicalDevice();
  if (!device_ || !device_->IsValid()) {
    FML_LOG(ERROR) << "Failed to create compatible logical device.";
    return;
  }

  // ---------------------------------------------------------------------------
  // Create a Skia context.
  // For creating SkSurfaces from VkImages and snapshotting them, etc.
  // ---------------------------------------------------------------------------

  uint32_t skia_features = 0;
  if (!device_->GetPhysicalDeviceFeaturesSkia(&skia_features)) {
    FML_LOG(ERROR) << "Failed to get physical device features.";

    return;
  }

  auto get_proc = vk_->CreateSkiaGetProc();
  if (get_proc == nullptr) {
    FML_LOG(ERROR) << "Failed to create Vulkan getProc for Skia.";
    return;
  }

  GrVkExtensions extensions;

  GrVkBackendContext backend_context = {};
  backend_context.fInstance = application_->GetInstance();
  backend_context.fPhysicalDevice = device_->GetPhysicalDeviceHandle();
  backend_context.fDevice = device_->GetHandle();
  backend_context.fQueue = device_->GetQueueHandle();
  backend_context.fGraphicsQueueIndex = device_->GetGraphicsQueueIndex();
  backend_context.fMinAPIVersion = VK_MAKE_VERSION(1, 0, 0);
  backend_context.fMaxAPIVersion = VK_MAKE_VERSION(1, 0, 0);
  backend_context.fFeatures = skia_features;
  backend_context.fVkExtensions = &extensions;
  backend_context.fGetProc = get_proc;
  backend_context.fOwnsInstanceAndDevice = false;

  GrContextOptions options =
      MakeDefaultContextOptions(ContextType::kRender, GrBackendApi::kVulkan);
  options.fReduceOpsTaskSplitting = GrContextOptions::Enable::kNo;
  context_ = GrDirectContext::MakeVulkan(backend_context, options);
}

TestVulkanContext::~TestVulkanContext() {
  context_->releaseResourcesAndAbandonContext();
}

std::optional<TestVulkanImage> TestVulkanContext::CreateImage(
    const SkISize& size) const {
  TestVulkanImage result;

  VkImageCreateInfo info = {
      .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
      .pNext = nullptr,
      .flags = 0,
      .imageType = VK_IMAGE_TYPE_2D,
      .format = VK_FORMAT_R8G8B8A8_UNORM,
      .extent = VkExtent3D{static_cast<uint32_t>(size.width()),
                           static_cast<uint32_t>(size.height()), 1},
      .mipLevels = 1,
      .arrayLayers = 1,
      .samples = VK_SAMPLE_COUNT_1_BIT,
      .tiling = VK_IMAGE_TILING_OPTIMAL,
      .usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT |
               VK_IMAGE_USAGE_TRANSFER_DST_BIT |
               VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
      .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
      .queueFamilyIndexCount = 0,
      .pQueueFamilyIndices = nullptr,
      .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
  };

  VkImage image;
  if (VK_CALL_LOG_ERROR(VK_CALL_LOG_ERROR(
          vk_->CreateImage(device_->GetHandle(), &info, nullptr, &image)))) {
    return std::nullopt;
  }

  result.image_ = vulkan::VulkanHandle<VkImage>(
      image, [&vk = vk_, &device = device_](VkImage image) {
        vk->DestroyImage(device->GetHandle(), image, nullptr);
      });

  VkMemoryRequirements mem_req;
  vk_->GetImageMemoryRequirements(device_->GetHandle(), image, &mem_req);
  VkMemoryAllocateInfo alloc_info{};
  alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  alloc_info.allocationSize = mem_req.size;
  alloc_info.memoryTypeIndex = static_cast<uint32_t>(__builtin_ctz(
      mem_req.memoryTypeBits & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT));

  VkDeviceMemory memory;
  if (VK_CALL_LOG_ERROR(vk_->AllocateMemory(device_->GetHandle(), &alloc_info,
                                            nullptr, &memory)) != VK_SUCCESS) {
    return std::nullopt;
  }

  result.memory_ = vulkan::VulkanHandle<VkDeviceMemory>{
      memory, [&vk = vk_, &device = device_](VkDeviceMemory memory) {
        vk->FreeMemory(device->GetHandle(), memory, nullptr);
      }};

  if (VK_CALL_LOG_ERROR(VK_CALL_LOG_ERROR(vk_->BindImageMemory(
          device_->GetHandle(), result.image_, result.memory_, 0)))) {
    return std::nullopt;
  }

  result.context_ =
      fml::RefPtr<TestVulkanContext>(const_cast<TestVulkanContext*>(this));

  return result;
}

sk_sp<GrDirectContext> TestVulkanContext::GetGrDirectContext() const {
  return context_;
}

}  // namespace testing
}  // namespace flutter
