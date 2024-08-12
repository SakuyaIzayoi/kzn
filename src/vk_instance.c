#include <stdlib.h>
#include <vulkan/vulkan_core.h>
#include "vk_fun.h"

VkInstance createInstance(SDL_Window *window) {
    char app_name[VK_MAX_EXTENSION_NAME_SIZE];
    strcpy(app_name, "kazan");
    char app_engine_name[VK_MAX_EXTENSION_NAME_SIZE];
    strcpy(app_engine_name, "kazan");

    VkApplicationInfo app_info = {
        VK_STRUCTURE_TYPE_APPLICATION_INFO,
        VK_NULL_HANDLE,
        app_name,
        VK_MAKE_VERSION(0,0,1),
        app_engine_name,
        VK_MAKE_VERSION(0,0,1),
        VK_API_VERSION_1_0
    };

	const char layer_list[][VK_MAX_EXTENSION_NAME_SIZE] = {
		"VK_LAYER_KHRONOS_validation"
	};
	const char *layers[] = {
		layer_list[0]
	};

	uint32_t extension_count= 0;
    SDL_Vulkan_GetInstanceExtensions(window, &extension_count, NULL);
    const char **extensions = (const char **)malloc(extension_count * sizeof(char *));
    SDL_Vulkan_GetInstanceExtensions(window, &extension_count, extensions);

	VkInstanceCreateInfo instanceCreateInfo = {
		VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
		VK_NULL_HANDLE,
		0,
		&app_info,
		1,
		layers,
		extension_count,
		extensions
	};

	VkInstance instance;
	vkCreateInstance(&instanceCreateInfo, VK_NULL_HANDLE, &instance);
	return instance;
}

void deleteInstance(VkInstance *instance) {
    vkDestroyInstance(*instance, VK_NULL_HANDLE);
}
