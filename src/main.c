#include <SDL2/SDL.h>
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_log.h>
#include <SDL2/SDL_vulkan.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

#include "sdl_fun.h"
#include "vk_fun.h"

int main(int argc, char **argv) {
    SDL_LogSetAllPriority(SDL_LOG_PRIORITY_DEBUG);
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) != 0) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "SDL_Init Error: %s", SDL_GetError());
        return EXIT_FAILURE;
    }
    SDL_Vulkan_LoadLibrary(NULL);

    SDL_Window *window = createVulkanWindow(640, 480, "Kzn");
    if (window == VK_NULL_HANDLE) {
        SDL_Vulkan_UnloadLibrary();
        SDL_Quit();

        return EXIT_FAILURE;
    }

    VkInstance instance = createInstance(window);

    uint32_t physical_device_count = getPhysicalDeviceCount(&instance);
    VkPhysicalDevice *physical_devices = getPhysicalDevices(&instance, physical_device_count);
    uint32_t best_physical_device_index = getBestPhysicalDeviceIndex(physical_devices, physical_device_count);
    VkPhysicalDevice *physical_device = &(physical_devices[best_physical_device_index]);

    uint32_t queue_family_count = getQueueFamilyCount(physical_device);
    VkQueueFamilyProperties *queue_family_properties = getQueueFamilyProperties(physical_device, queue_family_count);

    VkDevice device = createDevice(physical_device, queue_family_count, queue_family_properties);

    uint32_t best_graphics_queue_family_index = getBestGraphicsQueueFamilyIndex(queue_family_properties, queue_family_count);
    uint32_t graphics_queue_mode = getGraphicsQueueMode(queue_family_properties, best_graphics_queue_family_index);
    VkQueue drawing_queue = getDrawingQueue(&device, best_graphics_queue_family_index);
    VkQueue presenting_queue = getPresentingQueue(&device, best_graphics_queue_family_index, graphics_queue_mode);
    deleteQueueFamilyProperties(&queue_family_properties);

    VkSurfaceKHR surface = createSurface(window, instance);
    VkBool32 surface_supported = getSurfaceSupport(&surface, physical_device, best_graphics_queue_family_index);

    if (!surface_supported) {
        SDL_LogDebug(SDL_LOG_CATEGORY_VIDEO, "Vulkan surface not supported!\n");
        deleteSurface(&surface, &instance);
        deleteWindow(window);
        deleteDevice(&device);
        deletePhysicalDevices(&physical_devices);
        deleteInstance(&instance);

        SDL_Vulkan_UnloadLibrary();
        SDL_Quit();

        return EXIT_FAILURE;
    }

    VkSurfaceCapabilitiesKHR surface_capabilities = getSurfaceCapabilities(&surface, physical_device);
    VkSurfaceFormatKHR best_surface_format = getBestSurfaceFormat(&surface, physical_device);
    VkPresentModeKHR best_present_mode = getBestPresentMode(&surface, physical_device);
    VkExtent2D best_swapchain_extent = getBestSwapchainExtent(&surface_capabilities, window);
    uint32_t image_array_layers = 1;
    VkSwapchainKHR swapchain = createSwapchain(&device, &surface, &surface_capabilities, &best_surface_format,
                                               &best_swapchain_extent, &best_present_mode,
                                               image_array_layers, graphics_queue_mode);

    uint32_t swapchain_image_count = getSwapchainImageCount(&device, &swapchain);
    VkImage *swapchain_images = getSwapchainImages(&device, &swapchain, swapchain_image_count);

    VkImageView *swapchain_image_views = createImageViews(&device, &swapchain_images, &best_surface_format,
                                                          swapchain_image_count, image_array_layers);

    VkRenderPass render_pass = createRenderPass(&device, &best_surface_format);
    VkFramebuffer *framebuffers = createFramebuffers(&device, &render_pass, &best_swapchain_extent,
                                                     &swapchain_image_views, swapchain_image_count);

    uint32_t vertex_shader_size = 0;
    char vertex_shader_filename[] = "vert.spv";
    char *vertex_shader_code = getShaderCode(vertex_shader_filename, &vertex_shader_size);
    if (vertex_shader_code == VK_NULL_HANDLE) {
        SDL_LogDebug(SDL_LOG_CATEGORY_VIDEO, "Can't find SPIR-V file: %s", vertex_shader_filename);
        deleteFramebuffers(&device, &framebuffers, swapchain_image_count);
        deleteRenderPass(&device, &render_pass);
        deleteImageViews(&device, &swapchain_image_views, swapchain_image_count);
        deleteSwapchainImages(&swapchain_images);
        deleteSwapchain(&device, &swapchain);
        deleteSurface(&surface, &instance);
        deleteWindow(window);
        deleteDevice(&device);
        deletePhysicalDevices(&physical_devices);
        deleteInstance(&instance);

        SDL_Vulkan_UnloadLibrary();
        SDL_Quit();

        return EXIT_FAILURE;
    }
    VkShaderModule vertex_shader_module = createShaderModule(&device, vertex_shader_code, vertex_shader_size);

    uint32_t fragment_shader_size = 0;
    char fragment_shader_filename[] = "frag.spv";
    char *fragment_shader_code = getShaderCode(fragment_shader_filename, &fragment_shader_size);
    if (fragment_shader_code == VK_NULL_HANDLE) {
        SDL_LogDebug(SDL_LOG_CATEGORY_VIDEO, "Can't find SPIR-V file: %s", fragment_shader_filename);
        deleteFramebuffers(&device, &framebuffers, swapchain_image_count);
        deleteRenderPass(&device, &render_pass);
        deleteImageViews(&device, &swapchain_image_views, swapchain_image_count);
        deleteSwapchainImages(&swapchain_images);
        deleteSwapchain(&device, &swapchain);
        deleteSurface(&surface, &instance);
        deleteWindow(window);
        deleteDevice(&device);
        deletePhysicalDevices(&physical_devices);
        deleteInstance(&instance);

        SDL_Vulkan_UnloadLibrary();
        SDL_Quit();

        return 1;
    }
    VkShaderModule fragment_shader_module = createShaderModule(&device, fragment_shader_code, fragment_shader_size);

    VkPipelineLayout pipeline_layout = createPipelineLayout(&device);
    VkPipeline pipeline = createGraphicsPipeline(&device, &pipeline_layout, &vertex_shader_module, &fragment_shader_module, &render_pass, &best_swapchain_extent);

    deleteShaderModule(&device, &fragment_shader_module);
    deleteShaderCode(&fragment_shader_code);
    deleteShaderModule(&device, &vertex_shader_module);
    deleteShaderCode(&vertex_shader_code);

    VkCommandPool command_pool = createCommandPool(&device, best_graphics_queue_family_index);
    VkCommandBuffer *command_buffers = createCommandBuffers(&device, &command_pool, swapchain_image_count);
    recordCommandBuffers(&command_buffers, &render_pass, &framebuffers, &best_swapchain_extent, &pipeline, swapchain_image_count);

    uint32_t max_frames = 2;
    VkSemaphore *wait_semaphores = createSemaphores(&device, max_frames);
    VkSemaphore *signal_semaphores = createSemaphores(&device, max_frames);
    VkFence *front_fences = createFences(&device, max_frames);
    VkFence *back_fences = createEmptyFences(swapchain_image_count);

    presentImage(
        &device,
        window,
        command_buffers,
        front_fences,
        back_fences,
        wait_semaphores,
        signal_semaphores,
        &swapchain,
        &drawing_queue,
        &presenting_queue,
        max_frames);

    deleteEmptyFences(&back_fences);
    deleteFences(&device, &front_fences, max_frames);
    deleteSemaphores(&device, &signal_semaphores, max_frames);
    deleteSemaphores(&device, &wait_semaphores, max_frames);
    deleteCommandBuffers(&device, &command_buffers, &command_pool, swapchain_image_count);
    deleteCommandPool(&device, &command_pool);
    deleteGraphicsPipeline(&device, &pipeline);
    deletePipelineLayout(&device, &pipeline_layout);
    deleteFramebuffers(&device, &framebuffers, swapchain_image_count);
    deleteRenderPass(&device, &render_pass);
    deleteImageViews(&device, &swapchain_image_views, swapchain_image_count);
    deleteSwapchainImages(&swapchain_images);
    deleteSwapchain(&device, &swapchain);
    deleteSurface(&surface, &instance);
    deleteWindow(window);
    deleteDevice(&device);
    deletePhysicalDevices(&physical_devices);
    deleteInstance(&instance);

    SDL_Vulkan_UnloadLibrary();
    SDL_Quit();

    return EXIT_SUCCESS;
}
