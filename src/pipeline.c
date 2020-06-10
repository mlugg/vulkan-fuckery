#include <stdlib.h>
#include <vulkan/vulkan.h>
#include "common.h"
#include "pipeline.h"

VkResult _create_shader_module(const uint32_t *code, size_t code_size, VkShaderModule *module) {
  VkShaderModuleCreateInfo create_info = {
    .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
    .codeSize = code_size,
    .pCode = code
  };

  return vkCreateShaderModule(g_device, &create_info, NULL, module);
}

void pipeline_render_pass_create(void) {
  VkResult err;

  VkAttachmentDescription color_attachment = {
    .format = g_swapchain_image_format,
    .samples = VK_SAMPLE_COUNT_1_BIT,
    .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
    .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
    .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
    .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
    .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
    .finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
  };

  VkAttachmentReference color_attachment_ref = {
    .attachment = 0,
    .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
  };

  VkSubpassDescription subpass = {
    .colorAttachmentCount = 1,
    .pColorAttachments = &color_attachment_ref,
  };

  VkSubpassDependency dependency = {
    .srcSubpass = VK_SUBPASS_EXTERNAL,
    .dstSubpass = 0,
    .srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
    .srcAccessMask = 0,
    .dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
    .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
  };

  VkRenderPassCreateInfo create_info = {
    .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
    .attachmentCount = 1,
    .pAttachments = &color_attachment,
    .subpassCount = 1,
    .pSubpasses = &subpass,
    .dependencyCount = 1,
    .pDependencies = &dependency,
  };

  if ((err = vkCreateRenderPass(g_device, &create_info, NULL, &g_render_pass))) {
    ERR("Failed to create render pass: error %d", err);
  }
}

size_t read_file(const char *filename, uint32_t **data) {
  FILE *f = fopen(filename, "r");
  fseek(f, 0, SEEK_END);
  long size = ftell(f);
  fseek(f, 0, SEEK_SET);
  *data = malloc(size + 1);
  fread(*data, 4, size / 4, f);
  fclose(f);
  (*data)[size] = 0;
  return size;
}

void pipeline_create(void) {
  VkResult err;

  VkPipelineLayoutCreateInfo layout_info = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
  };

  if ((err = vkCreatePipelineLayout(g_device, &layout_info, NULL, &g_pipeline_layout))) {
    ERR("Failed to create pipeline layout: error %d", err);
  }

  uint32_t *vert_code, *frag_code;

  size_t vert_size = read_file("./shaders/vert.spv", &vert_code);
  size_t frag_size = read_file("./shaders/frag.spv", &frag_code);

  VkShaderModule vert_shader, frag_shader;
  if ((err = _create_shader_module(vert_code, vert_size, &vert_shader))) {
    ERR("Failed to create vertex shader module: error %d", err);
  }
  if ((err = _create_shader_module(frag_code, frag_size, &frag_shader))) {
    vkDestroyShaderModule(g_device, vert_shader, NULL);
    ERR("Failed to create fragment shader module: error %d", err);
  }

  VkPipelineShaderStageCreateInfo shader_stages[] = {
    {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
      .stage = VK_SHADER_STAGE_VERTEX_BIT,
      .module = vert_shader,
      .pName = "main",
    }, {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
      .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
      .module = frag_shader,
      .pName = "main",
    }
  };

  VkPipelineVertexInputStateCreateInfo vertex_input_state = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
  }; // TODO: actually input vertex data

  VkPipelineInputAssemblyStateCreateInfo input_assembly_state = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
    .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
    .primitiveRestartEnable = VK_FALSE,
  };

  VkViewport viewport = {
    .x = 0,
    .y = 0,
    .width = g_swapchain_extent.width,
    .height = g_swapchain_extent.height,
    .minDepth = 0.0f,
    .maxDepth = 1.0f,
  };

  VkRect2D scissor = {
    .offset = {0, 0},
    .extent = g_swapchain_extent,
  };

  VkPipelineViewportStateCreateInfo viewport_state = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
    .viewportCount = 1,
    .pViewports = &viewport,
    .scissorCount = 1,
    .pScissors = &scissor,
  };

  VkPipelineRasterizationStateCreateInfo rasterization_state = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
    .depthClampEnable = VK_FALSE,
    .rasterizerDiscardEnable = VK_FALSE,
    .polygonMode = VK_POLYGON_MODE_FILL,
    .lineWidth = 1.0f,
    .cullMode = VK_CULL_MODE_BACK_BIT,
    .frontFace = VK_FRONT_FACE_CLOCKWISE,
    .depthBiasEnable = VK_FALSE,
  };

  VkPipelineMultisampleStateCreateInfo multisample_state = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
    .sampleShadingEnable = VK_FALSE,
    .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
  };

  VkPipelineColorBlendAttachmentState color_blend_attachment = {
    .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
    .blendEnable = VK_FALSE,
  };

  VkPipelineColorBlendStateCreateInfo color_blend_state = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
    .logicOpEnable = VK_FALSE,
    .attachmentCount = 1,
    .pAttachments = &color_blend_attachment,
  };

  VkGraphicsPipelineCreateInfo create_info = {
    .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
    .stageCount = sizeof shader_stages / sizeof shader_stages[0],
    .pStages = shader_stages,
    .pVertexInputState = &vertex_input_state,
    .pInputAssemblyState = &input_assembly_state,
    .pViewportState = &viewport_state,
    .pRasterizationState = &rasterization_state,
    .pMultisampleState = &multisample_state,
    .pColorBlendState = &color_blend_state,
    .layout = g_pipeline_layout,
    .renderPass = g_render_pass,
    .subpass = 0,
  };

  if ((err = vkCreateGraphicsPipelines(g_device, NULL, 1, &create_info, NULL, &g_pipeline))) {
    vkDestroyShaderModule(g_device, vert_shader, NULL);
    vkDestroyShaderModule(g_device, frag_shader, NULL);
    ERR("Failed to create graphics pipeline: error %d", err);
  }

  vkDestroyShaderModule(g_device, vert_shader, NULL);
  vkDestroyShaderModule(g_device, frag_shader, NULL);
}
