#include <vulkan/vulkan.h>

#include "common.h"
#include "device.h"
#include "phys_device.h"

void device_create(void) {
  VkResult err;

  struct phys_device_queue_families queue_families = phys_device_get_queue_families();

  float queue_priority = 1.0f;

  const char *const ext_names[] = {
    "VK_KHR_swapchain",
  };

  size_t queue_count = (queue_families.graphics == queue_families.present) ? 1 : 2;

  VkDeviceQueueCreateInfo queue_infos[queue_count];

  queue_infos[0] = (VkDeviceQueueCreateInfo) {
    .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
    .queueFamilyIndex = queue_families.graphics,
    .queueCount = 1,
    .pQueuePriorities = &queue_priority,
  };

  if (queue_count > 1) {
    queue_infos[1] = (VkDeviceQueueCreateInfo) {
      .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
      .queueFamilyIndex = queue_families.present,
      .queueCount = 1,
      .pQueuePriorities = &queue_priority,
    };
  }

  VkDeviceCreateInfo create_info = {
    .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
    .queueCreateInfoCount = sizeof queue_infos / sizeof queue_infos[0],
    .pQueueCreateInfos = queue_infos,
    .enabledExtensionCount = sizeof ext_names / sizeof ext_names[0],
    .ppEnabledExtensionNames = ext_names,
  };

  if ((err = vkCreateDevice(g_phys_dev, &create_info, NULL, &g_device))) {
    ERR("Failed to create device: error %d", err);
  }

  vkGetDeviceQueue(g_device, queue_families.graphics, 0, &g_graphics_queue);
  vkGetDeviceQueue(g_device, queue_families.present, 0, &g_present_queue);
}
