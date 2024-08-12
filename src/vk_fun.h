#ifndef VK_FUN_H
#define VK_FUN_H

#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>
#include <SDL2/SDL_vulkan.h>

VkInstance createInstance(SDL_Window *);
void deleteInstance(VkInstance *);

#endif
