#include "flutter/flutter_vma/vulkan_extensions.h"

namespace flutter {

// finds the index of ext in infos or a negative result if ext is not found.
static int find_info(const std::vector<VulkanExtensions::Info>& infos,
                     const char ext[]) {
  if (infos.empty()) {
    return -1;
  }

  // TODO (kaushikiska): This really needs to be a binary search.
  for (size_t i = 0; i < infos.size(); i++) {
    if (strcmp(infos[i].fName.c_str(), ext) == 0) {
      return i;
    }
  }

  return -1;
}

namespace {  // This cannot be static because it is used as a template
             // parameter.
inline bool extension_compare(const VulkanExtensions::Info& a,
                              const VulkanExtensions::Info& b) {
  return strcmp(a.fName.c_str(), b.fName.c_str()) < 0;
}
}  // namespace

void VulkanExtensions::init(skgpu::VulkanGetProc getProc,
                            VkInstance instance,
                            VkPhysicalDevice physDev,
                            uint32_t instanceExtensionCount,
                            const char* const* instanceExtensions,
                            uint32_t deviceExtensionCount,
                            const char* const* deviceExtensions) {
  for (uint32_t i = 0; i < instanceExtensionCount; ++i) {
    const char* extension = instanceExtensions[i];
    // if not already in the list, add it
    if (find_info(extensions_, extension) < 0) {
      extensions_.push_back(Info(extension));
      std::sort(extensions_.begin(), extensions_.end(), extension_compare);
    }
  }
  for (uint32_t i = 0; i < deviceExtensionCount; ++i) {
    const char* extension = deviceExtensions[i];
    // if not already in the list, add it
    if (find_info(extensions_, extension) < 0) {
      extensions_.push_back(Info(extension));
      std::sort(extensions_.begin(), extensions_.end(), extension_compare);
    }
  }
  this->getSpecVersions(getProc, instance, physDev);
}

#define GET_PROC(F, inst) \
  PFN_vk##F grVk##F = (PFN_vk##F)getProc("vk" #F, inst, VK_NULL_HANDLE)

void VulkanExtensions::getSpecVersions(skgpu::VulkanGetProc getProc,
                                       VkInstance instance,
                                       VkPhysicalDevice physDevice) {
  // We grab all the extensions for the VkInstance and VkDevice so we can look
  // up what spec version each of the supported extensions are. We do not grab
  // the extensions for layers because we don't know what layers the client has
  // enabled and in general we don't do anything special for those extensions.

  if (instance == VK_NULL_HANDLE) {
    return;
  }
  GET_PROC(EnumerateInstanceExtensionProperties, VK_NULL_HANDLE);
  SkASSERT(grVkEnumerateInstanceExtensionProperties);

  VkResult res;
  // instance extensions
  uint32_t extensionCount = 0;
  res = grVkEnumerateInstanceExtensionProperties(nullptr, &extensionCount,
                                                 nullptr);
  if (VK_SUCCESS != res) {
    return;
  }
  VkExtensionProperties* extensions = new VkExtensionProperties[extensionCount];
  res = grVkEnumerateInstanceExtensionProperties(nullptr, &extensionCount,
                                                 extensions);
  if (VK_SUCCESS != res) {
    delete[] extensions;
    return;
  }
  for (uint32_t i = 0; i < extensionCount; ++i) {
    int idx = find_info(extensions_, extensions[i].extensionName);
    if (idx >= 0) {
      extensions_[idx].fSpecVersion = extensions[i].specVersion;
    }
  }
  delete[] extensions;

  if (physDevice == VK_NULL_HANDLE) {
    return;
  }
  GET_PROC(EnumerateDeviceExtensionProperties, instance);
  SkASSERT(grVkEnumerateDeviceExtensionProperties);

  // device extensions
  extensionCount = 0;
  res = grVkEnumerateDeviceExtensionProperties(physDevice, nullptr,
                                               &extensionCount, nullptr);
  if (VK_SUCCESS != res) {
    return;
  }
  extensions = new VkExtensionProperties[extensionCount];
  res = grVkEnumerateDeviceExtensionProperties(physDevice, nullptr,
                                               &extensionCount, extensions);
  if (VK_SUCCESS != res) {
    delete[] extensions;
    return;
  }
  for (uint32_t i = 0; i < extensionCount; ++i) {
    int idx = find_info(extensions_, extensions[i].extensionName);
    if (idx >= 0) {
      extensions_[idx].fSpecVersion = extensions[i].specVersion;
    }
  }
  delete[] extensions;
}

bool VulkanExtensions::hasExtension(const char ext[],
                                    uint32_t minVersion) const {
  int idx = find_info(extensions_, ext);
  return idx >= 0 && extensions_[idx].fSpecVersion >= minVersion;
}

}  // namespace flutter
