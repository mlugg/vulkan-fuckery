#include "common.h"

VkInstance g_inst;
VkPhysicalDevice g_phys_dev;
VkSurfaceKHR g_surface;
VkDevice g_device;
VkSwapchainKHR g_swapchain;
VkExtent2D g_swapchain_extent;
VkPipeline g_pipeline;
VkPipelineLayout g_pipeline_layout;
VkFormat g_swapchain_image_format;
VkRenderPass g_render_pass;
VkCommandPool g_command_pool;
VkQueue g_graphics_queue;
VkQueue g_present_queue;
struct sync_objects g_frame_sync[MAX_CONCURRENT_FRAMES];
struct swapchain_images g_swapchain_images;

struct vertex g_vertices[6] = {
  {{-0.5, -1.0}, {1.0, 0.0, 0.0}},
  {{ 0.0,  1.0}, {0.0, 1.0, 0.0}},
  {{-1.0,  1.0}, {0.0, 0.0, 1.0}},

  {{0.5, -1.0}, {1.0, 0.0, 0.0}},
  {{1.0,  1.0}, {0.0, 1.0, 0.0}},
  {{0.0,  1.0}, {0.0, 0.0, 1.0}},
};

VkDeviceMemory g_vertex_buffer_mem;
VkBuffer g_vertex_buffer;
