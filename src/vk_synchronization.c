#include "vk_fun.h"
#include <vulkan/vulkan_core.h>

VkSemaphore *createSemaphores(VkDevice *device, uint32_t max_frames) {
    VkSemaphoreCreateInfo semaphore_create_info = {
        VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
        VK_NULL_HANDLE,
        0};

    VkSemaphore *semaphore = (VkSemaphore *)malloc(max_frames * sizeof(VkSemaphore));
    for (uint32_t i = 0; i < max_frames; i++) {
        vkCreateSemaphore(*device, &semaphore_create_info, VK_NULL_HANDLE, &semaphore[i]);
    }
    return semaphore;
}

void deleteSemaphores(VkDevice *device, VkSemaphore **semaphores, uint32_t max_frames) {
    for (uint32_t i = 0; i < max_frames; i++) {
        vkDestroySemaphore(*device, (*semaphores)[i], VK_NULL_HANDLE);
    }
    free(*semaphores);
}

VkFence *createFences(VkDevice *device, uint32_t max_frames) {
    VkFenceCreateInfo fence_create_info = {
        VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
        VK_NULL_HANDLE,
        VK_FENCE_CREATE_SIGNALED_BIT};

    VkFence *fence = (VkFence *)malloc(max_frames * sizeof(VkFence));
    for (uint32_t i = 0; i < max_frames; i++) {
        vkCreateFence(*device, &fence_create_info, VK_NULL_HANDLE, &fence[i]);
    }
    return fence;
}

void deleteFences(VkDevice *device, VkFence **fences, uint32_t max_frames) {
    for (uint32_t i = 0; i < max_frames; i++) {
        vkDestroyFence(*device, (*fences)[i], VK_NULL_HANDLE);
    }
    free(*fences);
}

VkFence *createEmptyFences(uint32_t max_frames) {
    VkFence *fence = (VkFence *)malloc(max_frames * sizeof(VkFence));
    for (uint32_t i = 0; i < max_frames; i++) {
        fence[i] = VK_NULL_HANDLE;
    }
    return fence;
}

void deleteEmptyFences(VkFence **fences) {
    free(*fences);
}
