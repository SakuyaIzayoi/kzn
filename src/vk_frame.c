#include "vk_fun.h"
#include <vulkan/vulkan_core.h>

VkRenderPass createRenderPass(VkDevice *device, VkSurfaceFormatKHR *format) {
    VkAttachmentDescription attachment_description = {
        0,
        format->format,
        VK_SAMPLE_COUNT_1_BIT,
        VK_ATTACHMENT_LOAD_OP_CLEAR,
        VK_ATTACHMENT_STORE_OP_STORE,
        VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        VK_ATTACHMENT_STORE_OP_DONT_CARE,
        VK_IMAGE_LAYOUT_UNDEFINED,
        VK_IMAGE_LAYOUT_PRESENT_SRC_KHR};

    VkAttachmentReference attachment_reference = {
        0,
        VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};

    VkSubpassDescription subpass_description = {
        0,
        VK_PIPELINE_BIND_POINT_GRAPHICS,
        0,
        VK_NULL_HANDLE,
        1,
        &attachment_reference,
        VK_NULL_HANDLE,
        VK_NULL_HANDLE,
        0,
        VK_NULL_HANDLE};

    VkSubpassDependency subpass_dependency = {
        VK_SUBPASS_EXTERNAL,
        0,
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        0,
        VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
        0};

    VkRenderPassCreateInfo render_pass_create_info = {
        VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
        VK_NULL_HANDLE,
        0,
        1,
        &attachment_description,
        1,
        &subpass_description,
        1,
        &subpass_dependency};

    VkRenderPass render_pass;
    vkCreateRenderPass(*device, &render_pass_create_info, VK_NULL_HANDLE, &render_pass);
    SDL_LogDebug(SDL_LOG_CATEGORY_VIDEO, "Render pass created");
    return render_pass;
}

void deleteRenderPass(VkDevice *device, VkRenderPass *render_pass) {
    vkDestroyRenderPass(*device, *render_pass, VK_NULL_HANDLE);
}

VkFramebuffer *createFramebuffers(VkDevice *device, VkRenderPass *render_pass, VkExtent2D *extent,
                                  VkImageView **image_views, uint32_t image_view_count) {

    VkFramebufferCreateInfo *framebuffer_create_info = (VkFramebufferCreateInfo *)malloc(image_view_count * sizeof(VkFramebufferCreateInfo));
    VkFramebuffer *framebuffers = (VkFramebuffer *)malloc(image_view_count * sizeof(VkFramebuffer));

    for (uint32_t i = 0; i < image_view_count; i++) {
        framebuffer_create_info[i].sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebuffer_create_info[i].pNext = VK_NULL_HANDLE;
        framebuffer_create_info[i].flags = 0;
        framebuffer_create_info[i].renderPass = *render_pass;
        framebuffer_create_info[i].attachmentCount = 1;
        framebuffer_create_info[i].pAttachments = &(*image_views)[i];
        framebuffer_create_info[i].width = extent->width;
        framebuffer_create_info[i].height = extent->height;
        framebuffer_create_info[i].layers = 1;

        vkCreateFramebuffer(*device, &(framebuffer_create_info[i]),
                            NULL, &(framebuffers[i]));
        SDL_LogDebug(SDL_LOG_CATEGORY_VIDEO, "Framebuffer %d created", i);
    }

    free(framebuffer_create_info);
    return framebuffers;
}

void deleteFramebuffers(VkDevice *device, VkFramebuffer **framebuffers, uint32_t framebuffer_count) {
    for (uint32_t i = 0; i < framebuffer_count; i++) {
        vkDestroyFramebuffer(*device, (*framebuffers)[i], VK_NULL_HANDLE);
    }
    free(*framebuffers);
}
