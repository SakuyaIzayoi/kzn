#ifndef VK_FUN_H
#define VK_FUN_H

#include <SDL2/SDL_log.h>
#include <SDL2/SDL_vulkan.h>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

// vk_instance.c
VkInstance createInstance(SDL_Window *);
void deleteInstance(VkInstance *);

// vk_physicaldevice.c
uint32_t getPhysicalDeviceCount(VkInstance *);
VkPhysicalDevice *getPhysicalDevices(VkInstance *, uint32_t);
void deletePhysicalDevices(VkPhysicalDevice **);
uint32_t getBestPhysicalDeviceIndex(VkPhysicalDevice *, uint32_t);

// vk_queue.c
uint32_t getQueueFamilyCount(VkPhysicalDevice *);
VkQueueFamilyProperties *getQueueFamilyProperties(VkPhysicalDevice *, uint32_t);
void deleteQueueFamilyProperties(VkQueueFamilyProperties **);
uint32_t getBestGraphicsQueueFamilyIndex(VkQueueFamilyProperties *, uint32_t);
uint32_t getGraphicsQueueMode(VkQueueFamilyProperties *, uint32_t);
VkQueue getDrawingQueue(VkDevice *, uint32_t);
VkQueue getPresentingQueue(VkDevice *, uint32_t, uint32_t);

// vk_device.c
VkDevice createDevice(VkPhysicalDevice *, uint32_t, VkQueueFamilyProperties *);
void deleteDevice(VkDevice *);

// vk_surface.c
VkSurfaceKHR createSurface(SDL_Window *, VkInstance);
void deleteSurface(VkSurfaceKHR *, VkInstance *);
VkBool32 getSurfaceSupport(VkSurfaceKHR *, VkPhysicalDevice *, uint32_t);
VkSurfaceCapabilitiesKHR getSurfaceCapabilities(VkSurfaceKHR *, VkPhysicalDevice *);
VkSurfaceFormatKHR getBestSurfaceFormat(VkSurfaceKHR *, VkPhysicalDevice *);
VkPresentModeKHR getBestPresentMode(VkSurfaceKHR *, VkPhysicalDevice *);
VkExtent2D getBestSwapchainExtent(VkSurfaceCapabilitiesKHR *, SDL_Window *);
VkSwapchainKHR createSwapchain(VkDevice *, VkSurfaceKHR *, VkSurfaceCapabilitiesKHR *,
                               VkSurfaceFormatKHR *, VkExtent2D *, VkPresentModeKHR *,
                               uint32_t, uint32_t);
void deleteSwapchain(VkDevice *, VkSwapchainKHR *);

// vk_image.c
uint32_t getSwapchainImageCount(VkDevice *, VkSwapchainKHR *);
VkImage *getSwapchainImages(VkDevice *, VkSwapchainKHR *, uint32_t);
void deleteSwapchainImages(VkImage **);
VkImageView *createImageViews(VkDevice *, VkImage **, VkSurfaceFormatKHR *,
                              uint32_t, uint32_t);
void deleteImageViews(VkDevice *, VkImageView **, uint32_t);

// vk_frame.c
VkRenderPass createRenderPass(VkDevice *, VkSurfaceFormatKHR *);
void deleteRenderPass(VkDevice *, VkRenderPass *);
VkFramebuffer *createFramebuffers(VkDevice *, VkRenderPass *, VkExtent2D *, VkImageView **, uint32_t);
void deleteFramebuffers(VkDevice *, VkFramebuffer **, uint32_t);

// vk_shader.c
char *getShaderCode(const char *, uint32_t *);
void deleteShaderCode(char **);
VkShaderModule createShaderModule(VkDevice *, char *, uint32_t);
void deleteShaderModule(VkDevice *, VkShaderModule *);

// vk_pipeline.c
VkPipelineLayout createPipelineLayout(VkDevice *);
void deletePipelineLayout(VkDevice *, VkPipelineLayout *);
VkPipelineShaderStageCreateInfo configureVertexShaderStageCreateInfo(VkShaderModule *, const char *);
VkPipelineShaderStageCreateInfo configureFragmentShaderStageCreateInfo(VkShaderModule *, const char *);
VkPipelineVertexInputStateCreateInfo configureVertexInputStateCreateInfo(void);
VkPipelineInputAssemblyStateCreateInfo configureInputAssemblyStateCreateInfo(void);
VkViewport configureViewport(VkExtent2D *);
VkRect2D configureScissor(VkExtent2D *, uint32_t, uint32_t, uint32_t, uint32_t);
VkPipelineViewportStateCreateInfo configureViewportStateCreateInfo(VkViewport *, VkRect2D *);
VkPipelineRasterizationStateCreateInfo configureRasterizationStateCreateInfo(void);
VkPipelineMultisampleStateCreateInfo configureMultisampleStateCreateInfo(void);
VkPipelineColorBlendAttachmentState configureColorBlendAttachmentState(void);
VkPipelineColorBlendStateCreateInfo configureColorBlendStateCreateInfo(VkPipelineColorBlendAttachmentState *);
VkPipeline createGraphicsPipeline(
    VkDevice *,
    VkPipelineLayout *,
    VkShaderModule *,
    VkShaderModule *,
    VkRenderPass *,
    VkExtent2D *);
void deleteGraphicsPipeline(VkDevice *, VkPipeline *);

// vk_command.c
VkCommandPool createCommandPool(VkDevice *, uint32_t);
void deleteCommandPool(VkDevice *, VkCommandPool *);
VkCommandBuffer *createCommandBuffers(VkDevice *, VkCommandPool *, uint32_t);
void deleteCommandBuffers(VkDevice *, VkCommandBuffer **, VkCommandPool *, uint32_t);
void recordCommandBuffers(VkCommandBuffer **, VkRenderPass *, VkFramebuffer **, VkExtent2D *, VkPipeline *, uint32_t);

// vk_synchronization.c
VkSemaphore *createSemaphores(VkDevice *, uint32_t);
void deleteSemaphores(VkDevice *device, VkSemaphore **, uint32_t);
VkFence *createFences(VkDevice *, uint32_t);
void deleteFences(VkDevice *, VkFence **, uint32_t);
VkFence *createEmptyFences(uint32_t);
void deleteEmptyFences(VkFence **);

// vk_present.c
void presentImage(
    VkDevice *,
    SDL_Window *,
    VkCommandBuffer *,
    VkFence *,
    VkFence *,
    VkSemaphore *,
    VkSemaphore *,
    VkSwapchainKHR *,
    VkQueue *,
    VkQueue *,
    uint32_t);

#endif
