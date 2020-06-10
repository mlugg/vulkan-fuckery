#include <stdbool.h>
#include <vulkan/vulkan.h>
#include "phys_device.h"
#include "common.h"

void phys_device_select(void) {
  VkResult err;

  uint32_t count = 0;
  if ((err = vkEnumeratePhysicalDevices(g_inst, &count, NULL)) &&
      err != VK_INCOMPLETE) {
    ERR("Failed to get physical device count: error %d", err);
  }

  if (count == 0) ERR("No physical devices found");

  VkPhysicalDevice phys_devs[count];
  if ((err = vkEnumeratePhysicalDevices(g_inst, &count, phys_devs))) {
    ERR("Failed to enumerate physical devices: error %d", err);
  }

  g_phys_dev = phys_devs[0];
}

struct phys_device_queue_families phys_device_get_queue_families(void) {
  struct phys_device_queue_families families_out;

  uint32_t family_count;
  vkGetPhysicalDeviceQueueFamilyProperties(g_phys_dev, &family_count, NULL);

  VkQueueFamilyProperties families[family_count];
  vkGetPhysicalDeviceQueueFamilyProperties(g_phys_dev, &family_count, families);

  bool found_graphics = false, found_present = false;

  for (uint32_t i = 0; i < family_count; ++i) {
    if (!found_graphics && families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
      families_out.graphics = i;
      found_graphics = true;
    }

    VkBool32 present_support = VK_FALSE;
    vkGetPhysicalDeviceSurfaceSupportKHR(g_phys_dev, i, g_surface, &present_support);

    if (!found_present && present_support) {
      families_out.present = i;
      found_present = true;
    }

    if (found_graphics && found_present) {
      return families_out;
    }
  }

  ERR("Couldn't find required queue families");
}

VkSurfaceFormatKHR phys_device_choose_surface_format(void) {
  VkResult err;

  uint32_t format_count = 0;

  if ((err = vkGetPhysicalDeviceSurfaceFormatsKHR(g_phys_dev, g_surface, &format_count, NULL)) &&
      err != VK_INCOMPLETE) {
    ERR("Failed to get surface format count: error %d", err);
  }

  VkSurfaceFormatKHR formats[format_count];

  if ((err = vkGetPhysicalDeviceSurfaceFormatsKHR(g_phys_dev, g_surface, &format_count, formats))) {
    ERR("Failed to enumerate surface formats: error %d", err);
  }

  for (uint32_t i = 0; i < format_count; ++i) {
    if (formats[i].format != VK_FORMAT_B8G8R8A8_SRGB) continue;
    if (formats[i].colorSpace != VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) continue;
    return formats[i];
  }

  ERR("No suitable surface format found");
}

VkPresentModeKHR phys_device_choose_surface_present_mode(void) {
  VkResult err;

  uint32_t mode_count;

  if ((err = vkGetPhysicalDeviceSurfacePresentModesKHR(g_phys_dev, g_surface, &mode_count, NULL)) &&
      err != VK_INCOMPLETE) {
    ERR("Failed to get present mode count: error %d", err);
  }

  VkPresentModeKHR modes[mode_count];

  if ((err = vkGetPhysicalDeviceSurfacePresentModesKHR(g_phys_dev, g_surface, &mode_count, modes))) {
    ERR("Failed to enumerate present modes: error %d", err);
  }

  for (uint32_t i = 0; i < mode_count; ++i) {
    if (modes[i] == VK_PRESENT_MODE_MAILBOX_KHR) {
      return modes[i];
    }
  }

  // Guaranteed to be supported
  return VK_PRESENT_MODE_FIFO_KHR;
}
