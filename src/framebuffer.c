#include <vulkan/vulkan.h>

#include "common.h"
#include "framebuffer.h"

void framebuffer_create_all(void) {
  VkResult err;

  for (uint32_t i = 0; i < g_swapchain_images.count; ++i) {
    VkFramebufferCreateInfo create_info = {
      .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
      .renderPass = g_render_pass,
      .attachmentCount = 1,
      .pAttachments = &g_swapchain_images.images[i].image_view,
      .width = g_swapchain_extent.width,
      .height = g_swapchain_extent.height,
      .layers = 1,
    };

    if ((err = vkCreateFramebuffer(g_device, &create_info, NULL, &g_swapchain_images.images[i].framebuffer))) {
      ERR("Failed to create framebuffer %d: error %d", i, err);
    }
  }
}
