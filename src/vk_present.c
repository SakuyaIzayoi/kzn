#include "vk_fun.h"
#include <SDL2/SDL_events.h>
#include <vulkan/vulkan_core.h>

void presentImage(
    VkDevice *device,
    SDL_Window *window,
    VkCommandBuffer *command_buffers,
    VkFence *front_fences,
    VkFence *back_fences,
    VkSemaphore *wait_semaphores,
    VkSemaphore *signal_semaphores,
    VkSwapchainKHR *swapchain,
    VkQueue *draw_queue,
    VkQueue *present_queue,
    uint32_t max_frames) {
    uint32_t current_frame = 0;
    int running = 1;
    while (running) {
        SDL_Event event;
        while (SDL_PollEvent(&event))
            if (event.type == SDL_QUIT) {
                running = 0;
                break;
            }

        vkWaitForFences(*device, 1, &(front_fences[current_frame]), VK_TRUE, UINT64_MAX);
        uint32_t image_index = 0;
        vkAcquireNextImageKHR(*device, *swapchain, UINT64_MAX, wait_semaphores[current_frame], VK_NULL_HANDLE, &image_index);
        if (back_fences[image_index] != VK_NULL_HANDLE) {
            vkWaitForFences(*device, 1, &back_fences[image_index], VK_TRUE, UINT64_MAX);
        }
        back_fences[image_index] = front_fences[current_frame];

        VkPipelineStageFlags pipeline_stage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

        VkSubmitInfo submit_info = {
            VK_STRUCTURE_TYPE_SUBMIT_INFO,
            VK_NULL_HANDLE,
            1,
            &wait_semaphores[current_frame],
            &pipeline_stage,
            1,
            &command_buffers[image_index],
            1,
            &signal_semaphores[current_frame]};
        vkResetFences(*device, 1, &front_fences[current_frame]);
        vkQueueSubmit(*draw_queue, 1, &submit_info, front_fences[current_frame]);

        VkPresentInfoKHR present_info = {
            VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
            VK_NULL_HANDLE,
            1,
            &signal_semaphores[current_frame],
            1,
            &(*swapchain),
            &image_index,
            VK_NULL_HANDLE};
        vkQueuePresentKHR(*present_queue, &present_info);

        current_frame = (current_frame + 1) % max_frames;
    }

    vkDeviceWaitIdle(*device);
}
