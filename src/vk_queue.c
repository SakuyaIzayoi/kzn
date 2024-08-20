#include "vk_fun.h"
#include <vulkan/vulkan_core.h>

uint32_t getQueueFamilyCount(VkPhysicalDevice *device) {
    uint32_t queue_family_count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(*device, &queue_family_count, VK_NULL_HANDLE);
    return queue_family_count;
}

VkQueueFamilyProperties *getQueueFamilyProperties(VkPhysicalDevice *device, uint32_t queue_family_count) {
    VkQueueFamilyProperties *queue_family_properties = (VkQueueFamilyProperties *)malloc(queue_family_count * sizeof(VkQueueFamilyProperties));
    vkGetPhysicalDeviceQueueFamilyProperties(*device, &queue_family_count, queue_family_properties);
    return queue_family_properties;
}

void deleteQueueFamilyProperties(VkQueueFamilyProperties **properties) {
    free(*properties);
}

uint32_t getBestGraphicsQueueFamilyIndex(VkQueueFamilyProperties *queue_family_properties,
                                         uint32_t queue_family_count) {

    uint32_t graphics_queue_family_count = 0;
    uint32_t graphics_queue_family_indices[queue_family_count];

    for (uint32_t i = 0; i < queue_family_count; i++) {
        if ((queue_family_properties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0) {
            graphics_queue_family_indices[graphics_queue_family_count] = i;
            graphics_queue_family_count++;
        }
    }

    uint32_t queue_count = 0;
    uint32_t best_queue_index = 0;
    for (uint32_t i = 0; i < graphics_queue_family_count; i++) {
        if (queue_family_properties[graphics_queue_family_indices[i]].queueCount > queue_count) {
            queue_count = queue_family_properties[graphics_queue_family_indices[i]].queueCount;
            best_queue_index = graphics_queue_family_indices[i];
        }
    }
    SDL_LogDebug(SDL_LOG_CATEGORY_VIDEO, "Best queue family index: %d", best_queue_index);

    return best_queue_index;
}

uint32_t getGraphicsQueueMode(VkQueueFamilyProperties *family_queue_properties, uint32_t queue_family_index) {
    if (family_queue_properties[queue_family_index].queueCount == 1) {
        return 0;
    } else if (family_queue_properties[queue_family_index].queueCount > 1) {
        return 1;
    } else {
        return 2;
    }
}

VkQueue getDrawingQueue(VkDevice *device, uint32_t queue_family_index) {
    VkQueue queue = VK_NULL_HANDLE;
    vkGetDeviceQueue(*device, queue_family_index, 0, &queue);
    return queue;
}

VkQueue getPresentingQueue(VkDevice *device, uint32_t queue_family_index, uint32_t queue_mode) {
    VkQueue queue = VK_NULL_HANDLE;
    if (queue_mode == 0) {
        vkGetDeviceQueue(*device, queue_family_index, 0, &queue);
    } else if (queue_mode == 1) {
        vkGetDeviceQueue(*device, queue_family_index, 1, &queue);
    }
    return queue;
}
