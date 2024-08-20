#include "vk_fun.h"
#include <vulkan/vulkan_core.h>

VkPipelineLayout createPipelineLayout(VkDevice *device) {
    VkPipelineLayoutCreateInfo pipeline_layout_create_info = {
        VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        VK_NULL_HANDLE,
        0,
        0,
        VK_NULL_HANDLE,
        0,
        VK_NULL_HANDLE};

    VkPipelineLayout pipeline_layout;
    vkCreatePipelineLayout(*device, &pipeline_layout_create_info,
                           VK_NULL_HANDLE, &pipeline_layout);
    return pipeline_layout;
}

void deletePipelineLayout(VkDevice *device, VkPipelineLayout *layout) {
    vkDestroyPipelineLayout(*device, *layout, VK_NULL_HANDLE);
}

VkPipelineShaderStageCreateInfo configureVertexShaderStageCreateInfo(VkShaderModule *vertex_shader_module,
                                                                     const char *entry_name) {
    VkPipelineShaderStageCreateInfo vertex_shader_stage_create_info = {
        VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        VK_NULL_HANDLE,
        0,
        VK_SHADER_STAGE_VERTEX_BIT,
        *vertex_shader_module,
        entry_name,
        VK_NULL_HANDLE};

    return vertex_shader_stage_create_info;
}

VkPipelineShaderStageCreateInfo configureFragmentShaderStageCreateInfo(VkShaderModule *fragment_shader_module,
                                                                       const char *entry_name) {
    VkPipelineShaderStageCreateInfo fragment_shader_stage_create_info = {
        VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        VK_NULL_HANDLE,
        0,
        VK_SHADER_STAGE_FRAGMENT_BIT,
        *fragment_shader_module,
        entry_name,
        VK_NULL_HANDLE};

    return fragment_shader_stage_create_info;
}

VkPipelineVertexInputStateCreateInfo configureVertexInputStateCreateInfo(void) {
    VkPipelineVertexInputStateCreateInfo vertex_input_state_create_info = {
        VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
        VK_NULL_HANDLE,
        0,
        0,
        VK_NULL_HANDLE,
        0,
        VK_NULL_HANDLE};

    return vertex_input_state_create_info;
}

VkPipelineInputAssemblyStateCreateInfo configureInputAssemblyStateCreateInfo(void) {
    VkPipelineInputAssemblyStateCreateInfo input_assembly_state_create_info = {
        VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
        VK_NULL_HANDLE,
        0,
        VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
        VK_FALSE};

    return input_assembly_state_create_info;
}

VkViewport configureViewport(VkExtent2D *extent) {
    VkViewport viewport = {
        1.0f,
        1.0f,
        extent->width,
        extent->height,
        0.0f,
        1.0f};

    return viewport;
}

VkRect2D configureScissor(VkExtent2D *extent, uint32_t left, uint32_t right, uint32_t up, uint32_t down) {
    if (left > extent->width) {
        left = extent->width;
    }
    if (right > extent->width) {
        right = extent->width;
    }
    if (up > extent->height) {
        up = extent->height;
    }
    if (down > extent->height) {
        down = extent->height;
    }

    VkOffset2D offset = {
        left,
        up};

    VkExtent2D scissor_extent = {
        extent->width - left - right,
        extent->height - up - down};

    VkRect2D scissor = {
        offset,
        scissor_extent};

    return scissor;
}

VkPipelineViewportStateCreateInfo configureViewportStateCreateInfo(VkViewport *viewport, VkRect2D *scissor) {
    VkPipelineViewportStateCreateInfo viewport_state_create_info = {
        VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
        VK_NULL_HANDLE,
        0,
        1,
        viewport,
        1,
        scissor};

    return viewport_state_create_info;
}

VkPipelineRasterizationStateCreateInfo configureRasterizationStateCreateInfo(void) {
    VkPipelineRasterizationStateCreateInfo rasterization_state_create_info = {
        VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
        VK_NULL_HANDLE,
        0,
        VK_FALSE,
        VK_FALSE,
        VK_POLYGON_MODE_FILL,
        VK_CULL_MODE_BACK_BIT,
        VK_FRONT_FACE_CLOCKWISE,
        VK_FALSE,
        0.0f,
        0.0f,
        0.0f,
        1.0f};

    return rasterization_state_create_info;
}

VkPipelineMultisampleStateCreateInfo configureMultisampleStateCreateInfo(void) {
    VkPipelineMultisampleStateCreateInfo multisample_state_create_info = {
        VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
        VK_NULL_HANDLE,
        0,
        VK_SAMPLE_COUNT_1_BIT,
        VK_FALSE,
        1.0f,
        VK_NULL_HANDLE,
        VK_FALSE,
        VK_FALSE};

    return multisample_state_create_info;
}

VkPipelineColorBlendAttachmentState configureColorBlendAttachmentState(void) {
    VkPipelineColorBlendAttachmentState color_blend_attachment_state = {
        VK_FALSE,
        VK_BLEND_FACTOR_ONE,
        VK_BLEND_FACTOR_ZERO,
        VK_BLEND_OP_ADD,
        VK_BLEND_FACTOR_ONE,
        VK_BLEND_FACTOR_ZERO,
        VK_BLEND_OP_ADD,
        VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT};

    return color_blend_attachment_state;
}

VkPipelineColorBlendStateCreateInfo configureColorBlendStateCreateInfo(VkPipelineColorBlendAttachmentState *color_blend_attachment_state) {
    VkPipelineColorBlendStateCreateInfo color_blend_state_create_info = {
        VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
        VK_NULL_HANDLE,
        0,
        VK_FALSE,
        VK_LOGIC_OP_COPY,
        1,
        color_blend_attachment_state,
        {0.0f, 0.0f, 0.0f, 0.0f}};

    return color_blend_state_create_info;
}

VkPipeline createGraphicsPipeline(
    VkDevice *device,
    VkPipelineLayout *pipeline_layout,
    VkShaderModule *vertex_shader_module,
    VkShaderModule *fragment_shader_module,
    VkRenderPass *render_pass,
    VkExtent2D *extent) {
    char entry_name[] = "main";

    VkPipelineShaderStageCreateInfo shader_stage_create_info[] = {
        configureVertexShaderStageCreateInfo(vertex_shader_module, entry_name),
        configureFragmentShaderStageCreateInfo(fragment_shader_module, entry_name)};

    VkPipelineVertexInputStateCreateInfo vertex_input_state_create_info = configureVertexInputStateCreateInfo();
    VkPipelineInputAssemblyStateCreateInfo input_assembly_state_create_info = configureInputAssemblyStateCreateInfo();
    VkViewport viewport = configureViewport(extent);
    VkRect2D scissor = configureScissor(extent, 0, 0, 0, 0);
    VkPipelineViewportStateCreateInfo viewport_state_create_info = configureViewportStateCreateInfo(&viewport, &scissor);
    VkPipelineRasterizationStateCreateInfo rasterization_state_create_info = configureRasterizationStateCreateInfo();
    VkPipelineMultisampleStateCreateInfo multisample_state_create_info = configureMultisampleStateCreateInfo();
    VkPipelineColorBlendAttachmentState color_blend_attachment_state = configureColorBlendAttachmentState();
    VkPipelineColorBlendStateCreateInfo color_blend_state_create_info = configureColorBlendStateCreateInfo(&color_blend_attachment_state);

    VkGraphicsPipelineCreateInfo graphics_pipeline_create_info = {
        VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
        VK_NULL_HANDLE,
        0,
        2,
        shader_stage_create_info,
        &vertex_input_state_create_info,
        &input_assembly_state_create_info,
        VK_NULL_HANDLE,
        &viewport_state_create_info,
        &rasterization_state_create_info,
        &multisample_state_create_info,
        VK_NULL_HANDLE,
        &color_blend_state_create_info,
        VK_NULL_HANDLE,
        *pipeline_layout,
        *render_pass,
        0,
        VK_NULL_HANDLE,
        -1};

    VkPipeline graphics_pipeline;
    vkCreateGraphicsPipelines(*device, VK_NULL_HANDLE, 1, &graphics_pipeline_create_info, VK_NULL_HANDLE, &graphics_pipeline);
    SDL_LogDebug(SDL_LOG_CATEGORY_VIDEO, "Graphics pipeline created");
    return graphics_pipeline;
}

void deleteGraphicsPipeline(VkDevice *device, VkPipeline *pipeline) {
    vkDestroyPipeline(*device, *pipeline, VK_NULL_HANDLE);
}
