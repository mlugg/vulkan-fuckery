#ifndef DISPLAY_H
#define DISPLAY_H

#include <vulkan/vulkan.h>

struct display_plane_props { VkDisplayPlanePropertiesKHR plane; uint32_t idx; };

void display_select(VkDisplayPropertiesKHR *display);
void display_choose_mode(VkDisplayKHR display, VkDisplayModePropertiesKHR *mode);
void display_choose_plane(struct display_plane_props *plane);
void display_create_surface(VkDisplayModePropertiesKHR mode, struct display_plane_props plane);

#endif
