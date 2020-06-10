#include <time.h>
#include "render.h"

#include <vulkan/vulkan.h>

void render_create_sync_objects(void) {
  VkResult err;

  for (uint32_t i = 0; i < MAX_CONCURRENT_FRAMES; ++i) {
    VkSemaphoreCreateInfo semaphore_info = {
      .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
    };

    VkFenceCreateInfo fence_info = {
      .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
      .flags = VK_FENCE_CREATE_SIGNALED_BIT,
    };

    if ((err = vkCreateSemaphore(g_device, &semaphore_info, NULL, &g_frame_sync[i].image_available))) {
      ERR("Failed to create image_available semaphore %d: error %d", i, err);
    }

    if ((err = vkCreateSemaphore(g_device, &semaphore_info, NULL, &g_frame_sync[i].render_finished))) {
      ERR("Failed to create render_finished semaphore %d: error %d", i, err);
    }

    if ((err = vkCreateFence(g_device, &fence_info, NULL, &g_frame_sync[i].in_progress))) {
      ERR("Failed to create in_progress fence %d: error %d", i, err);
    }
  }

  for (uint32_t i = 0; i < g_swapchain_images.count; ++i) {
    g_swapchain_images.images[i].image_in_progress = VK_NULL_HANDLE;
  }
}

void render_loop(void) {
  VkResult err;

  uint32_t current_frame = 0;

  time_t end = time(NULL) + 15;

  while (time(NULL) < end) {
    vkWaitForFences(g_device, 1, &g_frame_sync[current_frame].in_progress, VK_TRUE, UINT64_MAX);
    vkResetFences(g_device, 1, &g_frame_sync[current_frame].in_progress);

    uint32_t img_idx;

    VkSemaphore wait_semaphores[] = { g_frame_sync[current_frame].image_available };
    VkSemaphore signal_semaphores[] = { g_frame_sync[current_frame].render_finished };

    vkAcquireNextImageKHR(g_device, g_swapchain, UINT64_MAX, wait_semaphores[0], VK_NULL_HANDLE, &img_idx);

    if (g_swapchain_images.images[img_idx].image_in_progress != VK_NULL_HANDLE) {
      vkWaitForFences(g_device, 1, &g_swapchain_images.images[img_idx].image_in_progress, VK_TRUE, UINT64_MAX);
    }

    g_swapchain_images.images[img_idx].image_in_progress = g_frame_sync[current_frame].in_progress;

    VkPipelineStageFlags wait_stages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};

    VkSubmitInfo submit_info = {
      .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
      .waitSemaphoreCount = sizeof wait_semaphores / sizeof wait_semaphores[0],
      .pWaitSemaphores = wait_semaphores,
      .pWaitDstStageMask = wait_stages,
      .commandBufferCount = 1,
      .pCommandBuffers = &g_swapchain_images.images[img_idx].draw_cmd_buffer,
      .signalSemaphoreCount = sizeof signal_semaphores / sizeof signal_semaphores[0],
      .pSignalSemaphores = signal_semaphores
    };

    if ((err = vkQueueSubmit(g_graphics_queue, 1, &submit_info, g_frame_sync[current_frame].in_progress))) {
      ERR("Failed to submit draw command buffer: eror %d", err);
    }

    VkPresentInfoKHR present_info = {
      .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
      .waitSemaphoreCount = 1,
      .pWaitSemaphores = signal_semaphores,
      .swapchainCount = 1,
      .pSwapchains = &g_swapchain,
      .pImageIndices = &img_idx,
    };

    vkQueuePresentKHR(g_present_queue, &present_info);

    current_frame = (current_frame + 1) % MAX_CONCURRENT_FRAMES;
  }

  vkDeviceWaitIdle(g_device);
}
