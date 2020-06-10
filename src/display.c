#include <vulkan/vulkan.h>
#include "display.h"
#include "common.h"

void display_select(VkDisplayPropertiesKHR *display) {
  VkResult err;

  uint32_t count = 0;
  if ((err = vkGetPhysicalDeviceDisplayPropertiesKHR(g_phys_dev, &count, NULL)) &&
      err != VK_INCOMPLETE) {
    ERR("Failed to get display count: error %d", err);
  }

  if (count == 0) ERR("No displays found");

  VkDisplayPropertiesKHR displays[count];
  if ((err = vkGetPhysicalDeviceDisplayPropertiesKHR(g_phys_dev, &count, displays))) {
    ERR("Failed to enumerate displays: error %d", err);
  }

  *display = displays[0];
}

void display_choose_mode(VkDisplayKHR display, VkDisplayModePropertiesKHR *mode) {
  VkResult err;

  uint32_t mode_count = 0;
  if ((err = vkGetDisplayModePropertiesKHR(g_phys_dev, display, &mode_count, NULL))) {
    ERR("Failed to get display mode count: error %d", err);
  }

  if (mode_count == 0) ERR("No display modes found");

  VkDisplayModePropertiesKHR modes[mode_count];
  if ((err = vkGetDisplayModePropertiesKHR(g_phys_dev, display, &mode_count, modes))) {
    ERR("Failed to enumerate display modes: error %d", err);
  }

  for (uint32_t i = 0; i < mode_count; ++i) {
    VkDisplayModeParametersKHR *params = &modes[i].parameters;
    LOG("Mode %d:", i);
    LOG("  Refresh rate: %f Hz", params->refreshRate / 1000.0f);
    LOG("  Visible region: %dx%d", params->visibleRegion.width, params->visibleRegion.height);
  }

  // TODO: Actually be fukin smart about it?
  *mode = modes[0];
}

void display_choose_plane(struct display_plane_props *props) {
  VkResult err;

  uint32_t plane_count = 0;
  if ((err = vkGetPhysicalDeviceDisplayPlanePropertiesKHR(g_phys_dev, &plane_count, NULL))) {
    ERR("Failed to get display plane count: error %d", err);
  }

  if (plane_count == 0) ERR("No display planes found");

  VkDisplayPlanePropertiesKHR planes[plane_count];
  if ((err = vkGetPhysicalDeviceDisplayPlanePropertiesKHR(g_phys_dev, &plane_count, planes))) {
    ERR("Failed to enumerate display planes: error %d", err);
  }

  for (uint32_t i = 0; i < plane_count; ++i) {
    if (planes[i].currentDisplay == VK_NULL_HANDLE) {
      props->idx = i;
      props->plane = planes[i];
      return;
    }
  }

  ERR("No display planes available");
}

void display_create_surface(VkDisplayModePropertiesKHR mode, struct display_plane_props plane) {
  VkResult err;

  VkDisplaySurfaceCreateInfoKHR surface_info = {
    .sType = VK_STRUCTURE_TYPE_DISPLAY_SURFACE_CREATE_INFO_KHR,
    .displayMode = mode.displayMode,
    .planeIndex = plane.idx,
    .planeStackIndex = plane.plane.currentStackIndex,
    .transform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR,
    .alphaMode = VK_DISPLAY_PLANE_ALPHA_OPAQUE_BIT_KHR,
    .imageExtent = mode.parameters.visibleRegion,
  };

  if ((err = vkCreateDisplayPlaneSurfaceKHR(g_inst, &surface_info, NULL, &g_surface))) {
    ERR("Failed to create surface for display plane: error %d", err);
  }
}
