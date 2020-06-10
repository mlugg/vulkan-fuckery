#include <string.h>
#include "buffer.h"
#include <vulkan/vulkan.h>

void buffer_init_vertex(void) {
  VkResult err;

  VkBufferCreateInfo buffer_info = {
    .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
    .size = sizeof g_vertices,
    .usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
    .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
  };

  if ((err = vkCreateBuffer(g_device, &buffer_info, NULL, &g_vertex_buffer))) {
    ERR("Failed to create vertex buffer: error %d", err);
  }

  VkMemoryRequirements reqs;
  vkGetBufferMemoryRequirements(g_device, g_vertex_buffer, &reqs);

  // find suitable memory type

  VkPhysicalDeviceMemoryProperties props;
  vkGetPhysicalDeviceMemoryProperties(g_phys_dev, &props);

  uint64_t mem_type_idx = UINT64_MAX;

  for (uint32_t i = 0; i < props.memoryTypeCount; ++i) {
    if (!(reqs.memoryTypeBits & (1<<i))) continue;
    if (!(props.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)) continue;
    if (!(props.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)) continue;
    mem_type_idx = i;
    break;
  }

  if (mem_type_idx == UINT64_MAX) {
    ERR("Failed to find a suitable memory type for vertex buffer");
  }

  VkMemoryAllocateInfo mem_info = {
    .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
    .allocationSize = reqs.size,
    .memoryTypeIndex = mem_type_idx,
  };

  if ((err = vkAllocateMemory(g_device, &mem_info, NULL, &g_vertex_buffer_mem))) {
    ERR("Failed to allocate memory for vertex buffer: error %d", err);
  }

  if ((err = vkBindBufferMemory(g_device, g_vertex_buffer, g_vertex_buffer_mem, 0))) {
    ERR("Failed to bind memory to vertex buffer: error %d", err);
  }

  void *mapped;

  if ((err = vkMapMemory(g_device, g_vertex_buffer_mem, 0, buffer_info.size, 0, &mapped))) {
    ERR("Failed to map vertex buffer memory: error %d", err);
  }

  memcpy(mapped, g_vertices, sizeof g_vertices);

  vkUnmapMemory(g_device, g_vertex_buffer_mem);
}
