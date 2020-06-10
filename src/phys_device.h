#ifndef PHYS_DEVICE_H
#define PHYS_DEVICE_H

#include <vulkan/vulkan.h>

struct phys_device_queue_families {
  uint32_t graphics;
  uint32_t present;
};

void phys_device_select(void);
struct phys_device_queue_families phys_device_get_queue_families(void);
VkSurfaceFormatKHR phys_device_choose_surface_format(void);
VkPresentModeKHR phys_device_choose_surface_present_mode(void);

#endif
