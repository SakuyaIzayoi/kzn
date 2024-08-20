#include "vk_fun.h"
#include <vulkan/vulkan_core.h>

char *getShaderCode(const char *filename, uint32_t *shader_size) {
    if (shader_size == VK_NULL_HANDLE) {
        return VK_NULL_HANDLE;
    }

    FILE *fp = VK_NULL_HANDLE;
    fp = fopen(filename, "rb+");
    if (fp == VK_NULL_HANDLE) {
        return VK_NULL_HANDLE;
    }
    fseek(fp, 0l, SEEK_END);
    *shader_size = (uint32_t)ftell(fp);
    rewind(fp);

    char *shader_code = (char *)malloc((*shader_size) * sizeof(char));
    fread(shader_code, 1, *shader_size, fp);

    fclose(fp);
    return shader_code;
}

void deleteShaderCode(char **shader_code) {
    free(*shader_code);
}

VkShaderModule createShaderModule(VkDevice *device, char *shader_code, uint32_t shader_size) {
    VkShaderModuleCreateInfo shader_module_create_info = {
        VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        VK_NULL_HANDLE,
        0,
        shader_size,
        (const uint32_t *)shader_code};

    VkShaderModule shader_module;
    vkCreateShaderModule(*device, &shader_module_create_info, VK_NULL_HANDLE, &shader_module);
    return shader_module;
}

void deleteShaderModule(VkDevice *device, VkShaderModule *shader_module) {
    vkDestroyShaderModule(*device, *shader_module, VK_NULL_HANDLE);
}
