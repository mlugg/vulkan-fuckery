#include <vulkan/vulkan.h>

#include "commands.h"
#include "common.h"
#include "phys_device.h"

void commands_create_pool(void) {
  VkResult err;

  struct phys_device_queue_families families = phys_device_get_queue_families();

  VkCommandPoolCreateInfo create_info = {
    .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
    .queueFamilyIndex = families.graphics,
  };

  if ((err = vkCreateCommandPool(g_device, &create_info, NULL, &g_command_pool))) {
    ERR("Failed to create command pool: error %d", err);
  }
}

void commands_create_buffers(void) {
  VkResult err;

  uint32_t count = g_swapchain_images.count;

  VkCommandBufferAllocateInfo alloc_info = {
    .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
    .commandPool = g_command_pool,
    .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
    .commandBufferCount = count,
  };

  VkCommandBuffer buffers[count];

  if ((err = vkAllocateCommandBuffers(g_device, &alloc_info, buffers))) {
    ERR("Failed to allocate command buffers: error %d", err);
  }

  for (uint32_t i = 0; i < count; ++i) {
    g_swapchain_images.images[i].draw_cmd_buffer = buffers[i];

    VkCommandBufferBeginInfo begin_info = {
      .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
    };

    if ((err = vkBeginCommandBuffer(buffers[i], &begin_info))) {
      ERR("Failed to record command buffer %d: error %d", i, err);
    }

    VkClearValue clear_color = {.color = {.float32 = {0.0f, 0.0f, 0.0f, 1.0f}}};

    VkRenderPassBeginInfo render_pass_info = {
      .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
      .renderPass = g_render_pass,
      .framebuffer = g_swapchain_images.images[i].framebuffer,
      .renderArea = {
        .offset = {0, 0},
        .extent = g_swapchain_extent,
      },
      .clearValueCount = 1,
      .pClearValues = &clear_color,
    };

    vkCmdBeginRenderPass(buffers[i], &render_pass_info, VK_SUBPASS_CONTENTS_INLINE);

    vkCmdBindPipeline(buffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, g_pipeline);

    VkDeviceSize offset = 0;
    vkCmdBindVertexBuffers(buffers[i], 0, 1, &g_vertex_buffer, &offset);

    vkCmdDraw(buffers[i], sizeof g_vertices / sizeof g_vertices[0], 1, 0, 0);

    vkCmdEndRenderPass(buffers[i]);

    if ((err = vkEndCommandBuffer(buffers[i]))) {
      ERR("Failed to end command buffer %d: error %d", i, err);
    }
  }
}
