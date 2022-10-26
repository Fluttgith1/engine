/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "flutter/flutter_vma/flutter_skia_vma.h"

#include "flutter/flutter_vma/vulkan_extensions.h"

namespace flutter {

sk_sp<skgpu::VulkanMemoryAllocator> FlutterSkiaVulkanMemoryAllocator::Make(
    VkInstance instance,
    VkPhysicalDevice physicalDevice,
    VkDevice device,
    uint32_t physicalDeviceVersion,
    const VulkanExtensions* extensions,
    sk_sp<const VulkanInterface> interface,
    bool mustUseCoherentHostVisibleMemory,
    bool threadSafe) {
#define SKGPU_COPY_FUNCTION(NAME) \
  functions.vk##NAME = interface->fFunctions.f##NAME
#define SKGPU_COPY_FUNCTION_KHR(NAME) \
  functions.vk##NAME##KHR = interface->fFunctions.f##NAME

  VmaVulkanFunctions functions;
  // We should be setting all the required functions (at least through
  // vulkan 1.1), but this is just extra belt and suspenders to make sure there
  // isn't unitialized values here.
  memset(&functions, 0, sizeof(VmaVulkanFunctions));

  // We don't use dynamic function getting in the allocator so we set the
  // getProc functions to null.
  functions.vkGetInstanceProcAddr = nullptr;
  functions.vkGetDeviceProcAddr = nullptr;
  SKGPU_COPY_FUNCTION(GetPhysicalDeviceProperties);
  SKGPU_COPY_FUNCTION(GetPhysicalDeviceMemoryProperties);
  SKGPU_COPY_FUNCTION(AllocateMemory);
  SKGPU_COPY_FUNCTION(FreeMemory);
  SKGPU_COPY_FUNCTION(MapMemory);
  SKGPU_COPY_FUNCTION(UnmapMemory);
  SKGPU_COPY_FUNCTION(FlushMappedMemoryRanges);
  SKGPU_COPY_FUNCTION(InvalidateMappedMemoryRanges);
  SKGPU_COPY_FUNCTION(BindBufferMemory);
  SKGPU_COPY_FUNCTION(BindImageMemory);
  SKGPU_COPY_FUNCTION(GetBufferMemoryRequirements);
  SKGPU_COPY_FUNCTION(GetImageMemoryRequirements);
  SKGPU_COPY_FUNCTION(CreateBuffer);
  SKGPU_COPY_FUNCTION(DestroyBuffer);
  SKGPU_COPY_FUNCTION(CreateImage);
  SKGPU_COPY_FUNCTION(DestroyImage);
  SKGPU_COPY_FUNCTION(CmdCopyBuffer);
  SKGPU_COPY_FUNCTION_KHR(GetBufferMemoryRequirements2);
  SKGPU_COPY_FUNCTION_KHR(GetImageMemoryRequirements2);
  SKGPU_COPY_FUNCTION_KHR(BindBufferMemory2);
  SKGPU_COPY_FUNCTION_KHR(BindImageMemory2);
  SKGPU_COPY_FUNCTION_KHR(GetPhysicalDeviceMemoryProperties2);

  VmaAllocatorCreateInfo info;
  info.flags = 0;
  if (!threadSafe) {
    info.flags |= VMA_ALLOCATOR_CREATE_EXTERNALLY_SYNCHRONIZED_BIT;
  }
  if (physicalDeviceVersion >= VK_MAKE_VERSION(1, 1, 0) ||
      (extensions->hasExtension(VK_KHR_DEDICATED_ALLOCATION_EXTENSION_NAME,
                                1) &&
       extensions->hasExtension(VK_KHR_GET_MEMORY_REQUIREMENTS_2_EXTENSION_NAME,
                                1))) {
    info.flags |= VMA_ALLOCATOR_CREATE_KHR_DEDICATED_ALLOCATION_BIT;
  }

  info.physicalDevice = physicalDevice;
  info.device = device;
  // 4MB was picked for the size here by looking at memory usage of Android apps
  // and runs of DM. It seems to be a good compromise of not wasting unused
  // allocated space and not making too many small allocations. The AMD
  // allocator will start making blocks at 1/8 the max size and builds up block
  // size as needed before capping at the max set here.
  info.preferredLargeHeapBlockSize = 4 * 1024 * 1024;
  info.pAllocationCallbacks = nullptr;
  info.pDeviceMemoryCallbacks = nullptr;
  info.pHeapSizeLimit = nullptr;
  info.pVulkanFunctions = &functions;
  info.instance = instance;
  // TODO (kaushikiska): Update our interface and headers to support vulkan 1.3
  // and add in the new required functions for 1.3 that the allocator needs.
  // Until then we just clamp the version to 1.1.
  info.vulkanApiVersion =
      std::min(physicalDeviceVersion, VK_MAKE_VERSION(1, 1, 0));
  info.pTypeExternalMemoryHandleTypes = nullptr;

  VmaAllocator allocator;
  vmaCreateAllocator(&info, &allocator);

  return sk_sp<FlutterSkiaVulkanMemoryAllocator>(
      new FlutterSkiaVulkanMemoryAllocator(allocator, std::move(interface),
                                           mustUseCoherentHostVisibleMemory));
}

FlutterSkiaVulkanMemoryAllocator::FlutterSkiaVulkanMemoryAllocator(
    VmaAllocator allocator,
    sk_sp<const VulkanInterface> interface,
    bool mustUseCoherentHostVisibleMemory)
    : allocator_(allocator),
      interface_(std::move(interface)),
      must_use_coherent_host_visible_memory_(mustUseCoherentHostVisibleMemory) {
}

FlutterSkiaVulkanMemoryAllocator::~FlutterSkiaVulkanMemoryAllocator() {
  vmaDestroyAllocator(allocator_);
  allocator_ = VK_NULL_HANDLE;
}

VkResult FlutterSkiaVulkanMemoryAllocator::allocateImageMemory(
    VkImage image,
    uint32_t allocationPropertyFlags,
    skgpu::VulkanBackendMemory* backendMemory) {
  VmaAllocationCreateInfo info;
  info.flags = 0;
  info.usage = VMA_MEMORY_USAGE_UNKNOWN;
  info.requiredFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
  info.preferredFlags = 0;
  info.memoryTypeBits = 0;
  info.pool = VK_NULL_HANDLE;
  info.pUserData = nullptr;

  if (kDedicatedAllocation_AllocationPropertyFlag & allocationPropertyFlags) {
    info.flags |= VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;
  }
  if (kLazyAllocation_AllocationPropertyFlag & allocationPropertyFlags) {
    info.requiredFlags |= VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT;
  }
  if (kProtected_AllocationPropertyFlag & allocationPropertyFlags) {
    info.requiredFlags |= VK_MEMORY_PROPERTY_PROTECTED_BIT;
  }

  VmaAllocation allocation;
  VkResult result =
      vmaAllocateMemoryForImage(allocator_, image, &info, &allocation, nullptr);
  if (VK_SUCCESS == result) {
    *backendMemory = reinterpret_cast<skgpu::VulkanBackendMemory>(allocation);
  }
  return result;
}

VkResult FlutterSkiaVulkanMemoryAllocator::allocateBufferMemory(
    VkBuffer buffer,
    BufferUsage usage,
    uint32_t allocationPropertyFlags,
    skgpu::VulkanBackendMemory* backendMemory) {
  VmaAllocationCreateInfo info;
  info.flags = 0;
  info.usage = VMA_MEMORY_USAGE_UNKNOWN;
  info.memoryTypeBits = 0;
  info.pool = VK_NULL_HANDLE;
  info.pUserData = nullptr;

  switch (usage) {
    case BufferUsage::kGpuOnly:
      info.requiredFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
      info.preferredFlags = 0;
      break;
    case BufferUsage::kCpuWritesGpuReads:
      // When doing cpu writes and gpu reads the general rule of thumb is to use
      // coherent memory. Though this depends on the fact that we are not doing
      // any cpu reads and the cpu writes are sequential. For sparse writes we'd
      // want cpu cached memory, however we don't do these types of writes in
      // Skia.
      //
      // TODO (kaushikiska): In the future there may be times where specific
      // types of memory could benefit from a coherent and cached memory.
      // Typically these allow for the gpu to read cpu writes from the cache
      // without needing to flush the writes throughout the cache. The reverse
      // is not true and GPU writes tend to invalidate the cache regardless.
      // Also these gpu cache read access are typically lower bandwidth than
      // non-cached memory. For now Skia doesn't really have a need or want of
      // this type of memory. But if we ever do we could pass in an
      // AllocationPropertyFlag that requests the cached property.
      info.requiredFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                           VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
      info.preferredFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
      break;
    case BufferUsage::kTransfersFromCpuToGpu:
      info.requiredFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                           VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
      info.preferredFlags = 0;
      break;
    case BufferUsage::kTransfersFromGpuToCpu:
      info.requiredFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
      info.preferredFlags = VK_MEMORY_PROPERTY_HOST_CACHED_BIT;
      break;
  }

  if (must_use_coherent_host_visible_memory_ &&
      (info.requiredFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)) {
    info.requiredFlags |= VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
  }
  if (kDedicatedAllocation_AllocationPropertyFlag & allocationPropertyFlags) {
    info.flags |= VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;
  }
  if ((kLazyAllocation_AllocationPropertyFlag & allocationPropertyFlags) &&
      BufferUsage::kGpuOnly == usage) {
    info.preferredFlags |= VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT;
  }

  if (kPersistentlyMapped_AllocationPropertyFlag & allocationPropertyFlags) {
    SkASSERT(BufferUsage::kGpuOnly != usage);
    info.flags |= VMA_ALLOCATION_CREATE_MAPPED_BIT;
  }

  VmaAllocation allocation;
  VkResult result = vmaAllocateMemoryForBuffer(allocator_, buffer, &info,
                                               &allocation, nullptr);
  if (VK_SUCCESS == result) {
    *backendMemory = reinterpret_cast<skgpu::VulkanBackendMemory>(allocation);
  }

  return result;
}

void FlutterSkiaVulkanMemoryAllocator::freeMemory(
    const skgpu::VulkanBackendMemory& memoryHandle) {
  const VmaAllocation allocation =
      reinterpret_cast<const VmaAllocation>(memoryHandle);
  vmaFreeMemory(allocator_, allocation);
}

void FlutterSkiaVulkanMemoryAllocator::getAllocInfo(
    const skgpu::VulkanBackendMemory& memoryHandle,
    skgpu::VulkanAlloc* alloc) const {
  const VmaAllocation allocation =
      reinterpret_cast<const VmaAllocation>(memoryHandle);
  VmaAllocationInfo vmaInfo;
  vmaGetAllocationInfo(allocator_, allocation, &vmaInfo);

  VkMemoryPropertyFlags memFlags;
  vmaGetMemoryTypeProperties(allocator_, vmaInfo.memoryType, &memFlags);

  uint32_t flags = 0;
  if (VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT & memFlags) {
    flags |= skgpu::VulkanAlloc::kMappable_Flag;
  }
  if (!SkToBool(VK_MEMORY_PROPERTY_HOST_COHERENT_BIT & memFlags)) {
    flags |= skgpu::VulkanAlloc::kNoncoherent_Flag;
  }
  if (VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT & memFlags) {
    flags |= skgpu::VulkanAlloc::kLazilyAllocated_Flag;
  }

  alloc->fMemory = vmaInfo.deviceMemory;
  alloc->fOffset = vmaInfo.offset;
  alloc->fSize = vmaInfo.size;
  alloc->fFlags = flags;
  alloc->fBackendMemory = memoryHandle;
}

VkResult FlutterSkiaVulkanMemoryAllocator::mapMemory(
    const skgpu::VulkanBackendMemory& memoryHandle,
    void** data) {
  const VmaAllocation allocation =
      reinterpret_cast<const VmaAllocation>(memoryHandle);
  return vmaMapMemory(allocator_, allocation, data);
}

void FlutterSkiaVulkanMemoryAllocator::unmapMemory(
    const skgpu::VulkanBackendMemory& memoryHandle) {
  const VmaAllocation allocation =
      reinterpret_cast<const VmaAllocation>(memoryHandle);
  vmaUnmapMemory(allocator_, allocation);
}

VkResult FlutterSkiaVulkanMemoryAllocator::flushMemory(
    const skgpu::VulkanBackendMemory& memoryHandle,
    VkDeviceSize offset,
    VkDeviceSize size) {
  const VmaAllocation allocation =
      reinterpret_cast<const VmaAllocation>(memoryHandle);
  return vmaFlushAllocation(allocator_, allocation, offset, size);
}

VkResult FlutterSkiaVulkanMemoryAllocator::invalidateMemory(
    const skgpu::VulkanBackendMemory& memoryHandle,
    VkDeviceSize offset,
    VkDeviceSize size) {
  const VmaAllocation allocation =
      reinterpret_cast<const VmaAllocation>(memoryHandle);
  return vmaInvalidateAllocation(allocator_, allocation, offset, size);
}

uint64_t FlutterSkiaVulkanMemoryAllocator::totalUsedMemory() const {
  VmaTotalStatistics stats;
  vmaCalculateStatistics(allocator_, &stats);
  return stats.total.statistics.allocationBytes;
}

uint64_t FlutterSkiaVulkanMemoryAllocator::totalAllocatedMemory() const {
  VmaTotalStatistics stats;
  vmaCalculateStatistics(allocator_, &stats);
  return stats.total.statistics.blockBytes;
}

}  // namespace flutter
