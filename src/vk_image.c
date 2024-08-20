#include "vk_fun.h"
#include <vulkan/vulkan_core.h>

uint32_t getSwapchainImageCount(VkDevice *device, VkSwapchainKHR *swapchain) {
    uint32_t swapchain_image_count;
    vkGetSwapchainImagesKHR(*device, *swapchain, &swapchain_image_count, VK_NULL_HANDLE);
    return swapchain_image_count;
}

VkImage *getSwapchainImages(VkDevice *device, VkSwapchainKHR *swapchain, uint32_t swapchain_image_count) {
    VkImage *swapchain_images = (VkImage *)malloc(swapchain_image_count * sizeof(VkImage));
    vkGetSwapchainImagesKHR(*device, *swapchain, &swapchain_image_count, swapchain_images);
    return swapchain_images;
}

void deleteSwapchainImages(VkImage **images) {
    free(*images);
}

VkImageView *createImageViews(VkDevice *device, VkImage **images, VkSurfaceFormatKHR *format,
                              uint32_t image_count, uint32_t image_array_layers) {

    VkComponentMapping component_mapping = {
        VK_COMPONENT_SWIZZLE_IDENTITY,
        VK_COMPONENT_SWIZZLE_IDENTITY,
        VK_COMPONENT_SWIZZLE_IDENTITY,
        VK_COMPONENT_SWIZZLE_IDENTITY};

    VkImageSubresourceRange image_subresource_range = {
        VK_IMAGE_ASPECT_COLOR_BIT,
        0,
        1,
        0,
        image_array_layers};

    VkImageViewCreateInfo *image_view_create_info = (VkImageViewCreateInfo *)malloc(image_count * sizeof(VkImageViewCreateInfo));
    VkImageView *image_views = (VkImageView *)malloc(image_count * sizeof(VkImageView));

    for (uint32_t i = 0; i < image_count; i++) {
        image_view_create_info[i].sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        image_view_create_info[i].pNext = VK_NULL_HANDLE;
        image_view_create_info[i].flags = 0;
        image_view_create_info[i].image = (*images)[i];
        image_view_create_info[i].viewType = VK_IMAGE_VIEW_TYPE_2D;
        image_view_create_info[i].format = format->format;
        image_view_create_info[i].components = component_mapping;
        image_view_create_info[i].subresourceRange = image_subresource_range;
        vkCreateImageView(*device, &image_view_create_info[i], VK_NULL_HANDLE, &image_views[i]);
        SDL_LogDebug(SDL_LOG_CATEGORY_VIDEO, "Image view %d created", i);
    }

    free(image_view_create_info);
    return image_views;
}

void deleteImageViews(VkDevice *device, VkImageView **image_views, uint32_t image_view_count) {
    for (uint32_t i = 0; i < image_view_count; i++) {
        vkDestroyImageView(*device, (*image_views)[i], VK_NULL_HANDLE);
    }
}
