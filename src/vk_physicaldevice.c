#include "vk_fun.h"
#include <vulkan/vulkan_core.h>

uint32_t getPhysicalDeviceCount(VkInstance *instance) {
    uint32_t device_count = 0;
    vkEnumeratePhysicalDevices(*instance, &device_count, VK_NULL_HANDLE);
    return device_count;
}

VkPhysicalDevice *getPhysicalDevices(VkInstance *instance, uint32_t device_count) {
    VkPhysicalDevice *devices = (VkPhysicalDevice *)malloc(device_count * sizeof(VkPhysicalDevice));
    vkEnumeratePhysicalDevices(*instance, &device_count, devices);
    return devices;
}

void deletePhysicalDevices(VkPhysicalDevice **devices) {
    free(*devices);
}

uint32_t getBestPhysicalDeviceIndex(VkPhysicalDevice *physical_devices, uint32_t device_count) {
    VkPhysicalDeviceProperties physicalDeviceProps[device_count];
    uint32_t discreteGPUList[device_count];
    uint32_t discreteGPUCount = 0;
    uint32_t integratedGPUList[device_count];
    uint32_t integratedGPUCount = 0;

    VkPhysicalDeviceMemoryProperties physicalDeviceMemProps[device_count];
    uint32_t physicalDeviceMemCount[device_count];
    VkDeviceSize physicalDeviceMemTotal[device_count];

    for (uint32_t i = 0; i < device_count; i++) {
        vkGetPhysicalDeviceProperties(physical_devices[i], &physicalDeviceProps[i]);
        if (physicalDeviceProps[i].deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
            discreteGPUList[discreteGPUCount] = i;
            discreteGPUCount++;
        } else if (physicalDeviceProps[i].deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU) {
            integratedGPUList[integratedGPUCount] = i;
            integratedGPUCount++;
        }

        vkGetPhysicalDeviceMemoryProperties(physical_devices[i], &physicalDeviceMemProps[i]);
        physicalDeviceMemCount[i] = physicalDeviceMemProps[i].memoryHeapCount;
        physicalDeviceMemTotal[i] = 0;
        for (uint32_t j = 0; j < physicalDeviceMemCount[i]; j++) {
            physicalDeviceMemTotal[i] += physicalDeviceMemProps[i].memoryHeaps[j].size;
        }
    }

    VkDeviceSize maxMemSize = 0;
    uint32_t best_device_index = 0;

    if (discreteGPUCount != 0) {
        for (uint32_t i = 0; i < discreteGPUCount; i++) {
            if (physicalDeviceMemTotal[i] > maxMemSize) {
                best_device_index = discreteGPUList[i];
                maxMemSize = physicalDeviceMemTotal[i];
            }
        }
    } else if (integratedGPUCount != 0) {
        for (uint32_t i = 0; i < integratedGPUCount; i++) {
            if (physicalDeviceMemTotal[i] > maxMemSize) {
                best_device_index = integratedGPUList[i];
                maxMemSize = physicalDeviceMemTotal[i];
            }
        }
    }

    SDL_LogDebug(SDL_LOG_CATEGORY_VIDEO, "Best device index: %u", best_device_index);
    SDL_LogDebug(SDL_LOG_CATEGORY_VIDEO, "Device name: %s", physicalDeviceProps[best_device_index].deviceName);
    if (discreteGPUCount != 0) {
        SDL_LogDebug(SDL_LOG_CATEGORY_VIDEO, "Device type: Discrete GPU");
    } else if (integratedGPUCount != 0) {
        SDL_LogDebug(SDL_LOG_CATEGORY_VIDEO, "Device type: Integrated GPU");
    } else {
        SDL_LogDebug(SDL_LOG_CATEGORY_VIDEO, "Device type: Unknown");
    }
    SDL_LogDebug(SDL_LOG_CATEGORY_VIDEO, "Total memory: %lu", physicalDeviceMemTotal[best_device_index]);

    return best_device_index;
}
