#ifndef COMMON_H
#define COMMON_H

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include <vulkan/vulkan.h>

#define _log(file, fmt, ...) fprintf(file, "%s:%d(%s): " fmt "%s", __FILE__, __LINE__, __func__, __VA_ARGS__)
#define LOG(...) _log(stdout, __VA_ARGS__, "\n")
#define ERR(...) (_log(stderr, __VA_ARGS__, "\n"), exit(1))

#define MAX_CONCURRENT_FRAMES 2

struct swapchain_images {
  uint32_t count;
  struct {
    VkImage image;
    VkImageView image_view;
    VkFramebuffer framebuffer;
    VkCommandBuffer draw_cmd_buffer;
    VkFence image_in_progress;
  } *images;
};

struct sync_objects {
  VkSemaphore image_available;
  VkSemaphore render_finished;
  VkFence in_progress;
};

extern VkInstance g_inst;
extern VkPhysicalDevice g_phys_dev;
extern VkSurfaceKHR g_surface;
extern VkDevice g_device;
extern VkSwapchainKHR g_swapchain;
extern VkExtent2D g_swapchain_extent;
extern VkPipeline g_pipeline;
extern VkPipelineLayout g_pipeline_layout;
extern VkFormat g_swapchain_image_format;
extern VkRenderPass g_render_pass;
extern VkCommandPool g_command_pool;
extern VkQueue g_graphics_queue;
extern VkQueue g_present_queue;
extern struct sync_objects g_frame_sync[MAX_CONCURRENT_FRAMES];
extern struct swapchain_images g_swapchain_images;

struct vertex {
  struct {
    float x, y;
  } pos;

  struct {
    float r, g, b;
  } color;
};

extern struct vertex g_vertices[6];

extern VkDeviceMemory g_vertex_buffer_mem;
extern VkBuffer g_vertex_buffer;

#endif
