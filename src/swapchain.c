#include <stdlib.h>
#include <vulkan/vulkan.h>

#include "common.h"
#include "swapchain.h"
#include "phys_device.h"

#define MIN(x, y) ((x) < (y) ? (x) : (y))
#define MAX(x, y) ((x) > (y) ? (x) : (y))

void swapchain_create(void) {
  VkResult err;

  VkSurfaceCapabilitiesKHR surf_caps;
  if ((err = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(g_phys_dev, g_surface, &surf_caps))) {
    ERR("Failed to get surface capabilities: error %d", err);
  }

  if (surf_caps.currentExtent.width != UINT32_MAX) {
    g_swapchain_extent = surf_caps.currentExtent;
  } else {
    g_swapchain_extent.width  = MAX(surf_caps.minImageExtent.width,  MIN(surf_caps.maxImageExtent.width,  1920));
    g_swapchain_extent.height = MAX(surf_caps.minImageExtent.height, MIN(surf_caps.maxImageExtent.height, 1080));
  }

  uint32_t image_count = surf_caps.minImageCount + 1;
  if (surf_caps.maxImageCount > 0) {
    image_count = MIN(surf_caps.maxImageCount, image_count);
  }

  VkSurfaceFormatKHR surf_format = phys_device_choose_surface_format();

  g_swapchain_image_format = surf_format.format;

  VkPresentModeKHR present_mode = phys_device_choose_surface_present_mode();

  struct phys_device_queue_families queue_families = phys_device_get_queue_families();

  uint32_t family_indices[] = { queue_families.graphics, queue_families.present };

  VkSwapchainCreateInfoKHR create_info = {
    .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
    .surface = g_surface,
    .minImageCount = image_count,
    .imageFormat = g_swapchain_image_format,
    .imageColorSpace = surf_format.colorSpace,
    .imageExtent = g_swapchain_extent,
    .imageArrayLayers = 1,
    .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
    .preTransform = surf_caps.currentTransform,
    .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
    .presentMode = present_mode,
    .clipped = VK_TRUE,
    .oldSwapchain = VK_NULL_HANDLE,
  };

  if (queue_families.graphics == queue_families.present) {
    create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
  } else {
    create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
    create_info.queueFamilyIndexCount = sizeof family_indices / sizeof family_indices[0];
    create_info.pQueueFamilyIndices = family_indices;
  }

  if ((err = vkCreateSwapchainKHR(g_device, &create_info, NULL, &g_swapchain))) {
    ERR("Failed to create swapchain: error %d", err);
  }

  vkGetSwapchainImagesKHR(g_device, g_swapchain, &image_count, NULL);
  g_swapchain_images.count = image_count;
  g_swapchain_images.images = malloc(image_count * sizeof g_swapchain_images.images[0]);
  VkImage images[image_count];
  vkGetSwapchainImagesKHR(g_device, g_swapchain, &image_count, images);
  for (uint32_t i = 0; i < image_count; ++i) {
    g_swapchain_images.images[i].image = images[i];

    VkImageViewCreateInfo view_info = {
      .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
      .image = images[i],
      .viewType = VK_IMAGE_VIEW_TYPE_2D,
      .format = g_swapchain_image_format,
      .components = {
        .r = VK_COMPONENT_SWIZZLE_IDENTITY,
        .g = VK_COMPONENT_SWIZZLE_IDENTITY,
        .b = VK_COMPONENT_SWIZZLE_IDENTITY,
        .a = VK_COMPONENT_SWIZZLE_IDENTITY,
      },
      .subresourceRange = {
        .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
        .baseMipLevel = 0,
        .levelCount = 1,
        .baseArrayLayer = 0,
        .layerCount = 1,
      },
    };

    if ((err = vkCreateImageView(g_device, &view_info, NULL, &g_swapchain_images.images[i].image_view))) {
      ERR("Failed to create image view %d: error %d", i, err);
    }
  }
}
