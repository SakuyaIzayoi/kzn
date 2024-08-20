#include "sdl_fun.h"
#include "vk_fun.h"
#include <SDL2/SDL_vulkan.h>
#include <vulkan/vulkan_core.h>

SDL_Window *createVulkanWindow(int width, int height, const char *title) {
    SDL_Window *window = SDL_CreateWindow(title, 100, 100, width, height, SDL_WINDOW_SHOWN | SDL_WINDOW_VULKAN);
    if (window == NULL) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "SDL_CreateWindow Error: %s", SDL_GetError());
        window = VK_NULL_HANDLE;
    }
    return window;
}

void deleteWindow(SDL_Window *window) {
    SDL_DestroyWindow(window);
}

VkSurfaceKHR createSurface(SDL_Window *window, VkInstance instance) {
    VkSurfaceKHR surface;
    SDL_Vulkan_CreateSurface(window, instance, &surface);
    return surface;
}

void deleteSurface(VkSurfaceKHR *surface, VkInstance *instance) {
    vkDestroySurfaceKHR(*instance, *surface, VK_NULL_HANDLE);
}

VkBool32 getSurfaceSupport(VkSurfaceKHR *surface, VkPhysicalDevice *device, uint32_t graphics_queue_family_index) {
    VkBool32 surface_support = 0;
    vkGetPhysicalDeviceSurfaceSupportKHR(*device, graphics_queue_family_index, *surface, &surface_support);
    return surface_support;
}

VkSurfaceCapabilitiesKHR getSurfaceCapabilities(VkSurfaceKHR *surface, VkPhysicalDevice *device) {
    VkSurfaceCapabilitiesKHR capabilities;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(*device, *surface, &capabilities);
    return capabilities;
}

VkSurfaceFormatKHR getBestSurfaceFormat(VkSurfaceKHR *surface, VkPhysicalDevice *device) {
    uint32_t surface_format_count;
    vkGetPhysicalDeviceSurfaceFormatsKHR(*device, *surface, &surface_format_count, VK_NULL_HANDLE);
    VkSurfaceFormatKHR *surface_formats = (VkSurfaceFormatKHR *)malloc(surface_format_count * sizeof(VkSurfaceFormatKHR));
    vkGetPhysicalDeviceSurfaceFormatsKHR(*device, *surface, &surface_format_count, surface_formats);
    VkSurfaceFormatKHR best_format = surface_formats[0];

    free(surface_formats);
    return best_format;
}

VkPresentModeKHR getBestPresentMode(VkSurfaceKHR *surface, VkPhysicalDevice *device) {
    uint32_t present_mode_count = 0;
    vkGetPhysicalDeviceSurfacePresentModesKHR(*device, *surface, &present_mode_count, VK_NULL_HANDLE);
    VkPresentModeKHR *present_modes = (VkPresentModeKHR *)malloc(present_mode_count * sizeof(VkPresentModeKHR));
    vkGetPhysicalDeviceSurfacePresentModesKHR(*device, *surface, &present_mode_count, present_modes);

    VkPresentModeKHR best_mode = VK_PRESENT_MODE_FIFO_KHR;

    for (uint32_t i = 0; i < present_mode_count; i++) {
        if (present_modes[i] == VK_PRESENT_MODE_MAILBOX_KHR) {
            best_mode = VK_PRESENT_MODE_MAILBOX_KHR;
        }
    }

    free(present_modes);
    return best_mode;
}

VkExtent2D getBestSwapchainExtent(VkSurfaceCapabilitiesKHR *capabilities, SDL_Window *window) {
    int fb_width = 0, fb_height = 0;
    SDL_Vulkan_GetDrawableSize(window, &fb_width, &fb_height);

    VkExtent2D best_swapchain_extent;
    best_swapchain_extent.height = (uint32_t)fb_height;

    if (capabilities->currentExtent.width < fb_width) {
        best_swapchain_extent.width = capabilities->currentExtent.width;
    } else {
        best_swapchain_extent.width = (uint32_t)fb_width;
    }

    if (capabilities->currentExtent.height < fb_height) {
        best_swapchain_extent.height = capabilities->currentExtent.height;
    } else {
        best_swapchain_extent.height = (uint32_t)fb_height;
    }

    return best_swapchain_extent;
}

VkSwapchainKHR createSwapchain(
    VkDevice *device,
    VkSurfaceKHR *surface,
    VkSurfaceCapabilitiesKHR *capabilities,
    VkSurfaceFormatKHR *format,
    VkExtent2D *extent,
    VkPresentModeKHR *present_mode,
    uint32_t image_array_layers,
    uint32_t graphics_queue_mode) {

    VkSharingMode image_sharing_mode = VK_SHARING_MODE_EXCLUSIVE;
    uint32_t queue_family_index_count = 0;
    uint32_t *queue_family_indices_ptr = VK_NULL_HANDLE;
    uint32_t queue_family_indices[] = {0, 1};
    if (graphics_queue_mode == 1) {
        image_sharing_mode = VK_SHARING_MODE_CONCURRENT;
        queue_family_index_count = 2;
        queue_family_indices_ptr = queue_family_indices;
    }

    VkSwapchainCreateInfoKHR swapchain_create_info = {
        VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        VK_NULL_HANDLE,
        0,
        *surface,
        capabilities->minImageCount + 1,
        format->format,
        format->colorSpace,
        *extent,
        image_array_layers,
        VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        image_sharing_mode,
        queue_family_index_count,
        queue_family_indices_ptr,
        capabilities->currentTransform,
        VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
        *present_mode,
        VK_TRUE,
        VK_NULL_HANDLE};

    VkSwapchainKHR swapchain;
    vkCreateSwapchainKHR(*device, &swapchain_create_info, VK_NULL_HANDLE, &swapchain);
    SDL_LogDebug(SDL_LOG_CATEGORY_VIDEO, "Swapchain created");
    return swapchain;
}

void deleteSwapchain(VkDevice *device, VkSwapchainKHR *swapchain) {
    vkDestroySwapchainKHR(*device, *swapchain, VK_NULL_HANDLE);
}
