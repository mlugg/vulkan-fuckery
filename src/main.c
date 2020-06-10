#include <vulkan/vulkan.h>
#include <stdio.h>
#include <stdlib.h>
#include "common.h"

#include "phys_device.h"
#include "display.h"
#include "device.h"
#include "swapchain.h"
#include "pipeline.h"
#include "framebuffer.h"
#include "commands.h"
#include "render.h"

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

static void _exit_cleanup(void) {
  for (uint32_t i = 0; i < MAX_CONCURRENT_FRAMES; ++i) {
    if (g_frame_sync[i].render_finished) vkDestroySemaphore(g_device, g_frame_sync[i].render_finished, NULL);
    if (g_frame_sync[i].image_available) vkDestroySemaphore(g_device, g_frame_sync[i].image_available, NULL);
    if (g_frame_sync[i].in_progress) vkDestroyFence(g_device, g_frame_sync[i].in_progress, NULL);
  }

  if (g_command_pool) vkDestroyCommandPool(g_device, g_command_pool, NULL);

  for (size_t i = 0; i < g_swapchain_images.count; ++i) {
    if (g_swapchain_images.images[i].framebuffer) vkDestroyFramebuffer(g_device, g_swapchain_images.images[i].framebuffer, NULL);
  }

  if (g_pipeline) vkDestroyPipeline(g_device, g_pipeline, NULL);
  if (g_pipeline_layout) vkDestroyPipelineLayout(g_device, g_pipeline_layout, NULL);
  if (g_render_pass) vkDestroyRenderPass(g_device, g_render_pass, NULL);

  for (size_t i = 0; i < g_swapchain_images.count; ++i) {
    if (g_swapchain_images.images[i].image_view) vkDestroyImageView(g_device, g_swapchain_images.images[i].image_view, NULL);
  }

  if (g_swapchain) vkDestroySwapchainKHR(g_device, g_swapchain, NULL);
  if (g_device) vkDestroyDevice(g_device, NULL);
  if (g_surface) vkDestroySurfaceKHR(g_inst, g_surface, NULL);
  if (g_inst) vkDestroyInstance(g_inst, NULL);
}

static void _init_inst(void) {
  VkResult err;

  VkApplicationInfo app_info = {
    .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
    .pApplicationName = "Vulkan Fuckery",
    .applicationVersion = VK_MAKE_VERSION(0, 1, 0),
    .pEngineName = "None",
    .engineVersion = 0,
    .apiVersion = VK_API_VERSION_1_0,
  };

  const char *exts[] = {
    "VK_KHR_surface",
    "VK_KHR_display",
  };

#ifndef NDEBUG
  const char *layers[] = {
    "VK_LAYER_KHRONOS_validation",
  };
#endif

  VkInstanceCreateInfo inst_info = {
    .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
    .pApplicationInfo = &app_info,
    .enabledExtensionCount = sizeof exts / sizeof exts[0],
    .ppEnabledExtensionNames = exts,
#ifndef NDEBUG
    .enabledLayerCount = sizeof layers / sizeof layers[0],
    .ppEnabledLayerNames = layers,
#else
    .enabledLayerCount = 0,
#endif
  };

  if ((err = vkCreateInstance(&inst_info, NULL, &g_inst))) {
    ERR("Failed to create Vulkan instance: error %d", err);
  }
}

int main(int argc, char **argv) {
  (void) argc;
  (void) argv;

  struct display_plane_props disp_plane;
  VkDisplayPropertiesKHR display;
  VkDisplayModePropertiesKHR display_mode;

  atexit(_exit_cleanup);

  _init_inst();

  phys_device_select();

  display_select(&display);

  display_choose_mode(display.display, &display_mode);

  display_choose_plane(&disp_plane);

  display_create_surface(display_mode, disp_plane);

  device_create();

  swapchain_create();

  pipeline_render_pass_create();

  pipeline_create();

  framebuffer_create_all();

  commands_create_pool();

  commands_create_buffers();

  render_create_sync_objects();

  render_loop();

  return 0;
}

