#include "vk_fun.h"
#include <vulkan/vulkan_core.h>

VkDevice createDevice(VkPhysicalDevice *device, uint32_t queue_family_count,
                      VkQueueFamilyProperties *queue_family_properties) {

    VkDeviceQueueCreateInfo *device_queue_create_info = (VkDeviceQueueCreateInfo *)malloc(queue_family_count * sizeof(VkDeviceQueueCreateInfo));
    float **queue_priorities = (float **)malloc(queue_family_count * sizeof(float *));

    for (uint32_t i = 0; i < queue_family_count; i++) {
        queue_priorities[i] = (float *)malloc(queue_family_properties[i].queueCount * sizeof(float));
        for (uint32_t j = 0; j < queue_family_properties[i].queueCount; j++) {
            queue_priorities[i][j] = 1.0f;
        }

        device_queue_create_info[i].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        device_queue_create_info[i].pNext = VK_NULL_HANDLE;
        device_queue_create_info[i].flags = 0;
        device_queue_create_info[i].queueFamilyIndex = i;
        device_queue_create_info[i].queueCount = queue_family_properties[i].queueCount;
        device_queue_create_info[i].pQueuePriorities = queue_priorities[i];
    }
    SDL_LogDebug(SDL_LOG_CATEGORY_VIDEO, "Using %d queue families", queue_family_count);

    const char extension_list[][VK_MAX_EXTENSION_NAME_SIZE] = {
        "VK_KHR_swapchain"};

    const char *extensions[] = {
        extension_list[0]};

    VkPhysicalDeviceFeatures device_features;
    vkGetPhysicalDeviceFeatures(*device, &device_features);

    VkDeviceCreateInfo device_create_info = {
        VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        VK_NULL_HANDLE,
        0,
        queue_family_count,
        device_queue_create_info,
        0,
        VK_NULL_HANDLE,
        1,
        extensions,
        &device_features};

    VkDevice vDevice;
    vkCreateDevice(*device, &device_create_info, VK_NULL_HANDLE, &vDevice);
    SDL_LogDebug(SDL_LOG_CATEGORY_VIDEO, "Logical device created");

    for (uint32_t i = 0; i < queue_family_count; i++) {
        free(queue_priorities[i]);
    }

    free(queue_priorities);
    free(device_queue_create_info);
    return vDevice;
}

void deleteDevice(VkDevice *device) {
    vkDestroyDevice(*device, VK_NULL_HANDLE);
}
