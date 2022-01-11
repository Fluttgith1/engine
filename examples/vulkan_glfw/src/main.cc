// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <cstdlib>
#include <iostream>
#include <optional>
#include <vector>

#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"

#include "embedder.h"  // Flutter's Embedder ABI.

static const bool g_enable_validation_layers = true;
static const size_t kInitialWindowWidth = 800;
static const size_t kInitialWindowHeight = 600;

static_assert(FLUTTER_ENGINE_VERSION == 1,
              "This Flutter Embedder was authored against the stable Flutter "
              "API at version 1. There has been a serious breakage in the "
              "API. Please read the ChangeLog and take appropriate action "
              "before updating this assertion");

/// Global struct for holding the Window+Vulkan state.
struct {
  GLFWwindow* window;
  VkInstance instance;
  VkSurfaceKHR surface;
  VkPhysicalDevice physical_device;
  std::vector<const char*> enabled_device_extensions;
  VkDevice device;
  uint32_t queue_family_index;
  VkQueue queue;
} g_state;

void GLFW_ErrorCallback(int error, const char* description) {
  std::cout << "GLFW Error: (" << error << ") " << description << std::endl;
}

void printUsage() {
  std::cout << "usage: flutter_glfw <path to project> <path to icudtl.dat>"
            << std::endl;
}

int main(int argc, char** argv) {
  if (argc != 3) {
    printUsage();
    return 1;
  }

  std::string project_path = argv[1];
  std::string icudtl_path = argv[2];

  /// --------------------------------------------------------------------------
  /// Create a GLFW Window.
  /// --------------------------------------------------------------------------

  glfwSetErrorCallback(GLFW_ErrorCallback);

  int result = glfwInit();
  assert(result == GLFW_TRUE);

  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  g_state.window = glfwCreateWindow(kInitialWindowWidth, kInitialWindowHeight,
                                    "Flutter", nullptr, nullptr);
  if (!g_state.window) {
    std::cerr << "Failed to create GLFW window." << std::endl;
    return EXIT_FAILURE;
  }

  /// --------------------------------------------------------------------------
  /// Create a Vulkan instance.
  /// --------------------------------------------------------------------------
  {
    uint32_t instance_extensions_count;
    const char** instance_extensions =
        glfwGetRequiredInstanceExtensions(&instance_extensions_count);

    std::cout << "GLFW requires " << instance_extensions_count
              << " Vulkan extensions:" << std::endl;
    for (int i = 0; i < instance_extensions_count; i++) {
      std::cout << " - " << instance_extensions[i] << std::endl;
    }

    VkApplicationInfo app_info = {
        .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .pNext = nullptr,
        .pApplicationName = "Flutter",
        .applicationVersion = VK_MAKE_VERSION(1, 0, 0),
        .pEngineName = "No Engine",
        .engineVersion = VK_MAKE_VERSION(1, 0, 0),
        .apiVersion = VK_MAKE_VERSION(1, 1, 0),
    };
    VkInstanceCreateInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    info.flags = 0;
    info.pApplicationInfo = &app_info;
    info.enabledExtensionCount = instance_extensions_count;
    info.ppEnabledExtensionNames = instance_extensions;
    if (g_enable_validation_layers) {
      uint32_t layer_count;
      vkEnumerateInstanceLayerProperties(&layer_count, nullptr);
      std::vector<VkLayerProperties> available_layers(layer_count);
      vkEnumerateInstanceLayerProperties(&layer_count, available_layers.data());

      const char* layer = "VK_LAYER_KHRONOS_validation";
      for (const auto& l : available_layers) {
        if (strcmp(l.layerName, layer) == 0) {
          info.enabledLayerCount = 1;
          info.ppEnabledLayerNames = &layer;
          break;
        }
      }
    }

    if (vkCreateInstance(&info, nullptr, &g_state.instance) != VK_SUCCESS) {
      std::cerr << "Failed to create Vulkan instance." << std::endl;
      return EXIT_FAILURE;
    }
  }

  /// --------------------------------------------------------------------------
  /// Create the window surface.
  /// --------------------------------------------------------------------------
  if (glfwCreateWindowSurface(g_state.instance, g_state.window, NULL,
                              &g_state.surface) != VK_SUCCESS) {
    std::cerr << "Failed to create window surface." << std::endl;
    return EXIT_FAILURE;
  }

  /// --------------------------------------------------------------------------
  /// Select a compatible physical device.
  /// --------------------------------------------------------------------------
  {
    uint32_t count;
    vkEnumeratePhysicalDevices(g_state.instance, &count, nullptr);
    std::vector<VkPhysicalDevice> physical_devices(count);
    vkEnumeratePhysicalDevices(g_state.instance, &count,
                               physical_devices.data());

    std::cout << "Enumerating " << count << " physical device(s)." << std::endl;

    uint32_t selected_score = 0;
    for (const auto& pdevice : physical_devices) {
      VkPhysicalDeviceProperties properties;
      VkPhysicalDeviceFeatures features;
      vkGetPhysicalDeviceProperties(pdevice, &properties);
      vkGetPhysicalDeviceFeatures(pdevice, &features);

      std::cout << "Checking device: " << properties.deviceName << std::endl;

      uint32_t score = 0;
      std::vector<const char*> supported_extensions;

      uint32_t qfp_count;
      vkGetPhysicalDeviceQueueFamilyProperties(pdevice, &qfp_count, nullptr);
      std::vector<VkQueueFamilyProperties> qfp(qfp_count);
      vkGetPhysicalDeviceQueueFamilyProperties(pdevice, &qfp_count, qfp.data());
      std::optional<uint32_t> graphics_queue_family;
      for (uint32_t i = 0; i < qfp.size(); i++) {
        // Only pick graphics queues that can present to the surface.
        // Graphics queues that can't present are rare if not nonexistent, but
        // the spec allows for this, so check it anyways.
        VkBool32 surface_present_supported;
        vkGetPhysicalDeviceSurfaceSupportKHR(pdevice, i, g_state.surface,
                                             &surface_present_supported);

        if (!graphics_queue_family.has_value() &&
            qfp[i].queueFlags & VK_QUEUE_GRAPHICS_BIT &&
            surface_present_supported) {
          graphics_queue_family = i;
        }
      }

      // Skip physical devices that don't have a graphics queue.
      if (!graphics_queue_family.has_value()) {
        std::cout << "  - Skipping due to no suitable graphics queues."
                  << std::endl;
        continue;
      }

      // Prefer discrete GPUs.
      if (properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
        score += 1 << 30;
      }

      uint32_t extension_count;
      vkEnumerateDeviceExtensionProperties(pdevice, nullptr, &extension_count,
                                           nullptr);
      std::vector<VkExtensionProperties> available_extensions(extension_count);
      vkEnumerateDeviceExtensionProperties(pdevice, nullptr, &extension_count,
                                           available_extensions.data());
      for (const auto& available_extension : available_extensions) {
        // The spec requires VK_KHR_portability_subset be enabled whenever it's
        // available on a device. It's present on compatibility ICDs like
        // MoltenVK.
        if (strcmp("VK_KHR_portability_subset",
                   available_extension.extensionName) == 0) {
          supported_extensions.push_back("VK_KHR_portability_subset");
        }

        // Prefer GPUs that support VK_KHR_get_memory_requirements2.
        else if (strcmp(VK_KHR_GET_MEMORY_REQUIREMENTS_2_EXTENSION_NAME,
                        available_extension.extensionName) == 0) {
          score += 1 << 29;
          supported_extensions.push_back(
              VK_KHR_GET_MEMORY_REQUIREMENTS_2_EXTENSION_NAME);
        }
      }

      // Prefer GPUs with larger max texture sizes.
      score += properties.limits.maxImageDimension2D;

      if (selected_score < score) {
        std::cout << "  - This is the best device so far. Score: 0x" << std::hex
                  << score << std::dec << std::endl;

        selected_score = score;
        g_state.physical_device = pdevice;
        g_state.enabled_device_extensions = supported_extensions;
        g_state.queue_family_index = graphics_queue_family.value_or(
            std::numeric_limits<uint32_t>::max());
      }
    }

    if (g_state.physical_device == nullptr) {
      std::cerr << "Failed to find a compatible Vulkan physical device."
                << std::endl;
      return EXIT_FAILURE;
    }
  }

  /// --------------------------------------------------------------------------
  /// Create a logical device.
  /// --------------------------------------------------------------------------
  {
    VkPhysicalDeviceFeatures device_features = {};

    VkDeviceQueueCreateInfo graphics_queue = {};
    graphics_queue.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    graphics_queue.queueFamilyIndex = g_state.queue_family_index;
    graphics_queue.queueCount = 1;
    float priority = 1.0f;
    graphics_queue.pQueuePriorities = &priority;

    VkDeviceCreateInfo device_info = {};
    device_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    device_info.enabledExtensionCount =
        g_state.enabled_device_extensions.size();
    device_info.ppEnabledExtensionNames =
        g_state.enabled_device_extensions.data();
    device_info.pEnabledFeatures = &device_features;
    device_info.queueCreateInfoCount = 1;
    device_info.pQueueCreateInfos = &graphics_queue;

    if (vkCreateDevice(g_state.physical_device, &device_info, nullptr,
                       &g_state.device) != VK_SUCCESS) {
      std::cerr << "Failed to create Vulkan logical device." << std::endl;
      return EXIT_FAILURE;
    }
  }

  /// --------------------------------------------------------------------------
  /// Get queue handle.
  /// --------------------------------------------------------------------------

  vkGetDeviceQueue(g_state.device, g_state.queue_family_index, 0,
                   &g_state.queue);

  std::cout << "Success.\n";

  while (!glfwWindowShouldClose(g_state.window)) {
    glfwPollEvents();
  }

  vkDestroyDevice(g_state.device, nullptr);
  vkDestroySurfaceKHR(g_state.instance, g_state.surface, nullptr);
  vkDestroyInstance(g_state.instance, nullptr);

  glfwDestroyWindow(g_state.window);
  glfwTerminate();

  return 0;
}
