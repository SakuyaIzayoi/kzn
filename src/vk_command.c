#include "vk_fun.h"
#include <vulkan/vulkan_core.h>

VkCommandPool createCommandPool(VkDevice *device, uint32_t queue_family_index) {
    VkCommandPoolCreateInfo command_pool_create_info = {
        VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        VK_NULL_HANDLE,
        0,
        queue_family_index};

    VkCommandPool command_pool;
    vkCreateCommandPool(*device, &command_pool_create_info, VK_NULL_HANDLE, &command_pool);
    SDL_LogDebug(SDL_LOG_CATEGORY_VIDEO, "Command pool created");
    return command_pool;
}

void deleteCommandPool(VkDevice *device, VkCommandPool *command_pool) {
    vkDestroyCommandPool(*device, *command_pool, VK_NULL_HANDLE);
}

VkCommandBuffer *createCommandBuffers(VkDevice *device, VkCommandPool *command_pool, uint32_t command_buffer_count) {
    VkCommandBufferAllocateInfo command_buffer_allocate_info = {
        VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        VK_NULL_HANDLE,
        *command_pool,
        VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        command_buffer_count};

    VkCommandBuffer *command_buffers = (VkCommandBuffer *)malloc(command_buffer_count * sizeof(VkCommandBuffer));
    vkAllocateCommandBuffers(*device, &command_buffer_allocate_info, command_buffers);
    SDL_LogDebug(SDL_LOG_CATEGORY_VIDEO, "Allocated command buffers");
    return command_buffers;
}

void deleteCommandBuffers(VkDevice *device, VkCommandBuffer **command_buffers, VkCommandPool *command_pool, uint32_t command_buffer_count) {
    vkFreeCommandBuffers(*device, *command_pool, command_buffer_count, *command_buffers);
    free(*command_buffers);
}

void recordCommandBuffers(VkCommandBuffer **command_buffers, VkRenderPass *render_pass, VkFramebuffer **framebuffers,
                          VkExtent2D *extent, VkPipeline *pipeline, uint32_t command_buffer_count) {

    VkCommandBufferBeginInfo *command_buffer_begin_infos = (VkCommandBufferBeginInfo *)malloc(command_buffer_count * sizeof(VkCommandBufferBeginInfo));
    VkRenderPassBeginInfo *render_pass_begin_infos = (VkRenderPassBeginInfo *)malloc(command_buffer_count * sizeof(VkRenderPassBeginInfo));

    VkRect2D render_area = {
        {0, 0},
        {extent->width, extent->height}};
    VkClearValue clear_value = {{{0.1f, 0.1f, 0.1f, 0.0f}}};

    for (uint32_t i = 0; i < command_buffer_count; i++) {
        command_buffer_begin_infos[i].sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        command_buffer_begin_infos[i].pNext = VK_NULL_HANDLE;
        command_buffer_begin_infos[i].flags = 0;
        command_buffer_begin_infos[i].pInheritanceInfo = VK_NULL_HANDLE;

        render_pass_begin_infos[i].sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        render_pass_begin_infos[i].pNext = VK_NULL_HANDLE;
        render_pass_begin_infos[i].renderPass = *render_pass;
        render_pass_begin_infos[i].framebuffer = (*framebuffers)[i];
        render_pass_begin_infos[i].renderArea = render_area;
        render_pass_begin_infos[i].clearValueCount = 1;
        render_pass_begin_infos[i].pClearValues = &clear_value;

        vkBeginCommandBuffer((*command_buffers)[i], &command_buffer_begin_infos[i]);
        vkCmdBeginRenderPass((*command_buffers)[i], &render_pass_begin_infos[i], VK_SUBPASS_CONTENTS_INLINE);
        vkCmdBindPipeline((*command_buffers)[i], VK_PIPELINE_BIND_POINT_GRAPHICS, *pipeline);
        vkCmdDraw((*command_buffers)[i], 3, 1, 0, 0);
        vkCmdEndRenderPass((*command_buffers)[i]);
        vkEndCommandBuffer((*command_buffers)[i]);
        SDL_LogDebug(SDL_LOG_CATEGORY_VIDEO, "Command buffer drawing recorded");
    }

    free(render_pass_begin_infos);
    free(command_buffer_begin_infos);
}
