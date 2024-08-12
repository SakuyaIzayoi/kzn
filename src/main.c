#include <SDL2/SDL.h>
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_log.h>
#include <SDL2/SDL_vulkan.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

#include "vk_fun.h"

int main(int argc, char **argv) {
    SDL_LogSetAllPriority(SDL_LOG_PRIORITY_DEBUG);
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) != 0) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "SDL_Init Error: %s", SDL_GetError());
        return EXIT_FAILURE;
    }
    SDL_Vulkan_LoadLibrary(NULL);

    SDL_Window *window = SDL_CreateWindow("Kzn", 100, 100, 640, 360, SDL_WINDOW_SHOWN | SDL_WINDOW_VULKAN);
    if (window == NULL) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "SDL_CreateWindow Error: %s", SDL_GetError());
        return EXIT_FAILURE;
    }

    VkInstance vkInst = createInstance(window);

    uint32_t physicalDeviceCount;
    vkEnumeratePhysicalDevices(vkInst, &physicalDeviceCount, NULL);

    VkPhysicalDevice physicalDevices[physicalDeviceCount];
    vkEnumeratePhysicalDevices(vkInst, &physicalDeviceCount, physicalDevices);

    VkPhysicalDeviceProperties physicalDeviceProps[physicalDeviceCount];
    uint32_t discreteGPUList[physicalDeviceCount];
    uint32_t discreteGPUCount = 0;
    uint32_t integratedGPUList[physicalDeviceCount];
    uint32_t integratedGPUCount = 0;

    VkPhysicalDeviceMemoryProperties physicalDeviceMemProps[physicalDeviceCount];
    uint32_t physicalDeviceMemCount[physicalDeviceCount];
    VkDeviceSize physicalDeviceMemTotal[physicalDeviceCount];

    for (uint32_t i = 0; i < physicalDeviceCount; i++) {
        vkGetPhysicalDeviceProperties(physicalDevices[i], &physicalDeviceProps[i]);
        if (physicalDeviceProps[i].deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
            discreteGPUList[discreteGPUCount] = i;
            discreteGPUCount++;
        } else if (physicalDeviceProps[i].deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU) {
            integratedGPUList[integratedGPUCount] = i;
            integratedGPUCount++;
        }

        vkGetPhysicalDeviceMemoryProperties(physicalDevices[i], &physicalDeviceMemProps[i]);
        physicalDeviceMemCount[i] = physicalDeviceMemProps[i].memoryHeapCount;
        physicalDeviceMemTotal[i] = 0;
        for (uint32_t j = 0; j < physicalDeviceMemCount[i]; j++) {
            physicalDeviceMemTotal[i] += physicalDeviceMemProps[i].memoryHeaps[j].size;
        }
    }

    VkDeviceSize maxMemSize = 0;
    uint32_t physicalDeviceBestIndex = 0;

    if (discreteGPUCount != 0) {
        for (uint32_t i = 0; i < discreteGPUCount; i++) {
            if (physicalDeviceMemTotal[i] > maxMemSize) {
                physicalDeviceBestIndex = discreteGPUList[i];
                maxMemSize = physicalDeviceMemTotal[i];
            }
        }
    } else if (integratedGPUCount != 0) {
        for (uint32_t i = 0; i < integratedGPUCount; i++) {
            if (physicalDeviceMemTotal[i] > maxMemSize) {
                physicalDeviceBestIndex = integratedGPUList[i];
                maxMemSize = physicalDeviceMemTotal[i];
            }
        }
    }

    SDL_LogDebug(SDL_LOG_CATEGORY_VIDEO, "Best device index: %u", physicalDeviceBestIndex);
    SDL_LogDebug(SDL_LOG_CATEGORY_VIDEO, "Device name: %s", physicalDeviceProps[physicalDeviceBestIndex].deviceName);
    if (discreteGPUCount != 0) {
        SDL_LogDebug(SDL_LOG_CATEGORY_VIDEO, "Device type: discrete gpu");
    } else if (integratedGPUCount != 0) {
        SDL_LogDebug(SDL_LOG_CATEGORY_VIDEO, "Device type: integrated gpu");
    } else {
        SDL_LogDebug(SDL_LOG_CATEGORY_VIDEO, "Device type: unknown");
    }
    SDL_LogDebug(SDL_LOG_CATEGORY_VIDEO, "total memory: %lu", physicalDeviceMemTotal[physicalDeviceBestIndex]);

    VkPhysicalDevice *physicalDevice = &(physicalDevices[physicalDeviceBestIndex]);

    // Queue families
    uint32_t qfPropCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(*physicalDevice, &qfPropCount, NULL);
    VkQueueFamilyProperties qfProps[qfPropCount];
    vkGetPhysicalDeviceQueueFamilyProperties(*physicalDevice, &qfPropCount, qfProps);

    uint32_t qfQCount[qfPropCount];
    for (uint32_t i = 0; i < qfPropCount; i++) {
        qfQCount[i] = qfProps[i].queueCount;
    }

    // Logical Device
    VkDeviceQueueCreateInfo devQCreateInfos[qfPropCount];
    for (uint32_t i = 0; i < qfPropCount; i++) {
        devQCreateInfos[i].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        devQCreateInfos[i].pNext = NULL;
        devQCreateInfos[i].flags = 0;
        devQCreateInfos[i].queueFamilyIndex = i;
        devQCreateInfos[i].queueCount = qfQCount[i];
        float qPrior[1] = {1.0f};
        devQCreateInfos[i].pQueuePriorities = qPrior;
    }
    SDL_LogDebug(SDL_LOG_CATEGORY_VIDEO, "using %d queue families", qfPropCount);

    VkDeviceCreateInfo devCreateInfo;
    devCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    devCreateInfo.pNext = NULL;
    devCreateInfo.flags = 0;
    devCreateInfo.queueCreateInfoCount = qfPropCount;
    devCreateInfo.pQueueCreateInfos = devQCreateInfos;
    devCreateInfo.enabledLayerCount = 0;
    devCreateInfo.ppEnabledLayerNames = NULL;

    uint32_t devExtCount = 1;
    devCreateInfo.enabledExtensionCount = devExtCount;
    char ppDevExts[devExtCount][VK_MAX_EXTENSION_NAME_SIZE];
    strcpy(ppDevExts[0], "VK_KHR_swapchain");
    char *ppDevExtNames[devExtCount];
    for (uint32_t i = 0; i < devExtCount; i++) {
        ppDevExtNames[i] = ppDevExts[i];
    }
    devCreateInfo.ppEnabledExtensionNames = (const char *const *)ppDevExtNames;

    VkPhysicalDeviceFeatures physicalDeviceFeatures;
    vkGetPhysicalDeviceFeatures(*physicalDevice, &physicalDeviceFeatures);
    devCreateInfo.pEnabledFeatures = &physicalDeviceFeatures;

    VkDevice dev;
    vkCreateDevice(*physicalDevice, &devCreateInfo, NULL, &dev);
    SDL_LogDebug(SDL_LOG_CATEGORY_VIDEO, "logical device created");

    // Select best queue

    uint32_t qfGraphCount = 0;
    uint32_t qfGraphList[qfPropCount];
    for (uint32_t i = 0; i < qfPropCount; i++) {
        if ((qfProps[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0) {
            qfGraphList[qfGraphCount] = i;
            qfGraphCount++;
        }
    }

    uint32_t maxQCount = 0;
    uint32_t qfBestIndex = 0;
    for (uint32_t i = 0; i < qfGraphCount; i++) {
        if (qfProps[qfGraphList[i]].queueCount > maxQCount) {
            qfBestIndex = qfGraphList[i];
        }
    }
    SDL_LogDebug(SDL_LOG_CATEGORY_VIDEO, "Best queue family index: %d", qfBestIndex);

    VkQueue qGraph, qPres;
    vkGetDeviceQueue(dev, qfBestIndex, 0, &qGraph);
    char singleQueue = 1;
    if (qfProps[qfBestIndex].queueCount < 2) {
        vkGetDeviceQueue(dev, qfBestIndex, 0, &qPres);
        SDL_LogDebug(SDL_LOG_CATEGORY_VIDEO, "using single queue for drawing");
    } else {
        singleQueue = 0;
        vkGetDeviceQueue(dev, qfBestIndex, 1, &qPres);
        SDL_LogDebug(SDL_LOG_CATEGORY_VIDEO, "using double queues for drawing");
    }

    VkSurfaceKHR surf;
    SDL_Vulkan_CreateSurface(window, vkInst, &surf);

    // Swapchain and image view
    VkBool32 phySurfSupported;
    vkGetPhysicalDeviceSurfaceSupportKHR(*physicalDevice, qfBestIndex, surf, &phySurfSupported);

    if (phySurfSupported == VK_TRUE) {
        SDL_LogDebug(SDL_LOG_CATEGORY_VIDEO, "surface supported");
    } else {
        SDL_LogDebug(SDL_LOG_CATEGORY_VIDEO, "warning: surface unsupported!");
    }

    // Get surface caps
    VkSurfaceCapabilitiesKHR surfCaps;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(*physicalDevice, surf, &surfCaps);
    char extentSuitable = 1;
    int windW, windH;
    SDL_Vulkan_GetDrawableSize(window, &windW, &windH);
    VkExtent2D actualExtent;
    actualExtent.width = (uint32_t)windW;
    actualExtent.height = (uint32_t)windH;

    if (surfCaps.currentExtent.width != windW || surfCaps.currentExtent.height != windH) {
        extentSuitable = 0;
        SDL_LogDebug(SDL_LOG_CATEGORY_VIDEO, "actual extent size doesn't match framebuffers, resizing...");
        actualExtent.width =
            windW > surfCaps
                        .maxImageExtent.width
                ? surfCaps
                      .maxImageExtent.width
                : (uint32_t)windW;
        actualExtent.width =
            windW < surfCaps
                        .minImageExtent.width
                ? surfCaps
                      .minImageExtent.width
                : (uint32_t)windW;
        actualExtent.height =
            windH > surfCaps
                        .maxImageExtent.height
                ? surfCaps
                      .maxImageExtent.height
                : (uint32_t)windH;
        actualExtent.height =
            windH < surfCaps
                        .minImageExtent.height
                ? surfCaps
                      .minImageExtent.height
                : (uint32_t)windH;
    }

    // Fetch surface formats
    uint32_t surfFormCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(*physicalDevice, surf, &surfFormCount, NULL);
    VkSurfaceFormatKHR surfForms[surfFormCount];
    vkGetPhysicalDeviceSurfaceFormatsKHR(*physicalDevice, surf, &surfFormCount, surfForms);
    SDL_LogDebug(SDL_LOG_CATEGORY_VIDEO, "fetched %d surface formats", surfFormCount);
    for (uint32_t i = 0; i < surfFormCount; i++) {
        SDL_LogDebug(SDL_LOG_CATEGORY_VIDEO, "format: %d\tcolorspace: %d",
               surfForms[i].format,
               surfForms[i].colorSpace);
    }

    // Fetch surface present mode
    uint32_t presentModeCount = 0;
    vkGetPhysicalDeviceSurfacePresentModesKHR(*physicalDevice, surf, &presentModeCount, NULL);
    VkPresentModeKHR presentModes[presentModeCount];
    vkGetPhysicalDeviceSurfacePresentModesKHR(*physicalDevice, surf, &presentModeCount, presentModes);
    SDL_LogDebug(SDL_LOG_CATEGORY_VIDEO, "Fetched %d present modes", presentModeCount);
    char mailboxModeSupported = 0;
    for (uint32_t i = 0; i < presentModeCount; i++) {
        SDL_LogDebug(SDL_LOG_CATEGORY_VIDEO, "present mode: %d", presentModes[i]);
        if (presentModes[i] == VK_PRESENT_MODE_MAILBOX_KHR) {
            SDL_LogDebug(SDL_LOG_CATEGORY_VIDEO, "mailbox present mode supported");
            mailboxModeSupported = 1;
        }
    }

    // Create swapchain
    VkSwapchainCreateInfoKHR swapCreateInfo;
    swapCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapCreateInfo.pNext = NULL;
    swapCreateInfo.flags = 0;
    swapCreateInfo.surface = surf;
    swapCreateInfo.minImageCount = surfCaps.minImageCount + 1;
    swapCreateInfo.imageFormat = surfForms[0].format;
    swapCreateInfo.imageColorSpace = surfForms[0].colorSpace;
    swapCreateInfo.imageExtent = extentSuitable ? surfCaps.currentExtent : actualExtent;
    swapCreateInfo.imageArrayLayers = 1;
    swapCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    swapCreateInfo.imageSharingMode = singleQueue ? VK_SHARING_MODE_EXCLUSIVE : VK_SHARING_MODE_CONCURRENT;
    swapCreateInfo.queueFamilyIndexCount = singleQueue ? 0 : 2;
    uint32_t qfIndices[2] = {0, 1};
    swapCreateInfo.pQueueFamilyIndices = singleQueue ? NULL : qfIndices;
    swapCreateInfo.preTransform = surfCaps.currentTransform;
    swapCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    swapCreateInfo.presentMode = mailboxModeSupported ? VK_PRESENT_MODE_MAILBOX_KHR : VK_PRESENT_MODE_FIFO_KHR;
    swapCreateInfo.clipped = VK_TRUE;
    swapCreateInfo.oldSwapchain = VK_NULL_HANDLE;

    VkSwapchainKHR swap;
    vkCreateSwapchainKHR(dev, &swapCreateInfo, NULL, &swap);
    SDL_LogDebug(SDL_LOG_CATEGORY_VIDEO, "swapchain created");

    // Create image views

    uint32_t swapImageCount = 0;
    vkGetSwapchainImagesKHR(dev, swap, &swapImageCount, NULL);
    VkImage swapImages[swapImageCount];
    vkGetSwapchainImagesKHR(dev, swap, &swapImageCount, swapImages);
    SDL_LogDebug(SDL_LOG_CATEGORY_VIDEO, "%d images fetched from swapchain", swapImageCount);

    VkImageView imageViews[swapImageCount];
    VkImageViewCreateInfo imageViewCreateInfos[swapImageCount];

    VkComponentMapping imageViewRGBAComp;
    imageViewRGBAComp.r = VK_COMPONENT_SWIZZLE_IDENTITY;
    imageViewRGBAComp.g = VK_COMPONENT_SWIZZLE_IDENTITY;
    imageViewRGBAComp.b = VK_COMPONENT_SWIZZLE_IDENTITY;
    imageViewRGBAComp.a = VK_COMPONENT_SWIZZLE_IDENTITY;

    VkImageSubresourceRange imageViewSubres;
    imageViewSubres.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    imageViewSubres.baseMipLevel = 0;
    imageViewSubres.levelCount = 1;
    imageViewSubres.baseArrayLayer = 0;
    imageViewSubres.layerCount = swapCreateInfo.imageArrayLayers;

    for (uint32_t i = 0; i < swapImageCount; i++) {
        imageViewCreateInfos[i].sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        imageViewCreateInfos[i].pNext = NULL;
        imageViewCreateInfos[i].flags = 0;
        imageViewCreateInfos[i].image = swapImages[i];
        imageViewCreateInfos[i].viewType = VK_IMAGE_VIEW_TYPE_2D;
        imageViewCreateInfos[i].format = surfForms[0].format;
        imageViewCreateInfos[i].components = imageViewRGBAComp;
        imageViewCreateInfos[i].subresourceRange = imageViewSubres;
        vkCreateImageView(dev, &imageViewCreateInfos[i], NULL, &imageViews[i]);
        SDL_LogDebug(SDL_LOG_CATEGORY_VIDEO, "image view %d created", i);
    }

    // Create render pass

    VkAttachmentDescription attachDesc;
    attachDesc.flags = 0;
    attachDesc.format = swapCreateInfo.imageFormat;
    attachDesc.samples = VK_SAMPLE_COUNT_1_BIT;
    attachDesc.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachDesc.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachDesc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachDesc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachDesc.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attachDesc.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference attachRef;
    attachRef.attachment = 0;
    attachRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpDesc;
    subpDesc.flags = 0;
    subpDesc.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpDesc.inputAttachmentCount = 0;
    subpDesc.pInputAttachments = NULL;
    subpDesc.colorAttachmentCount = 1;
    subpDesc.pColorAttachments = &attachRef;
    subpDesc.pResolveAttachments = NULL;
    subpDesc.pDepthStencilAttachment = NULL;
    subpDesc.preserveAttachmentCount = 0;
    subpDesc.pPreserveAttachments = NULL;

    VkSubpassDependency subpDep;
    subpDep.srcSubpass = VK_SUBPASS_EXTERNAL;
    subpDep.dstSubpass = 0;
    subpDep.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    subpDep.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    subpDep.srcAccessMask = 0;
    subpDep.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    subpDep.dependencyFlags = 0;

    VkRenderPassCreateInfo rendpCreateInfo;
    rendpCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    rendpCreateInfo.pNext = NULL;
    rendpCreateInfo.flags = 0;
    rendpCreateInfo.attachmentCount = 1;
    rendpCreateInfo.pAttachments = &attachDesc;
    rendpCreateInfo.subpassCount = 1;
    rendpCreateInfo.pSubpasses = &subpDesc;
    rendpCreateInfo.dependencyCount = 1;
    rendpCreateInfo.pDependencies = &subpDep;

    VkRenderPass rendp;
    vkCreateRenderPass(dev, &rendpCreateInfo, NULL, &rendp);
    SDL_LogDebug(SDL_LOG_CATEGORY_VIDEO, "render pass created");

    // Create pipeline

    // Load Shaders
    FILE *fpVert = NULL;
    FILE *fpFrag = NULL;

    fpVert = fopen("vert.spv", "rb+");
    fpFrag = fopen("frag.spv", "rb+");
    char shaderLoaded = 1;

    if (fpVert == NULL || fpFrag == NULL) {
        shaderLoaded = 0;
        SDL_LogDebug(SDL_LOG_CATEGORY_VIDEO, "can't find SPIR-V files");
    }
    fseek(fpVert, 0, SEEK_END);
    fseek(fpFrag, 0, SEEK_END);
    uint32_t vertSize = (uint32_t)ftell(fpVert);
    uint32_t fragSize = (uint32_t)ftell(fpFrag);

    char *pVertCode = (char *)malloc(vertSize * sizeof(char));
    char *pFragCode = (char *)malloc(fragSize * sizeof(char));

    rewind(fpVert);
    rewind(fpFrag);

    if (fread(pVertCode, 1, vertSize, fpVert) != vertSize) {
        SDL_LogError(SDL_LOG_CATEGORY_VIDEO, "Error reading vertex shader");
        return EXIT_FAILURE;
    }
    if (fread(pFragCode, 1, fragSize, fpFrag) != fragSize) {
        SDL_LogError(SDL_LOG_CATEGORY_VIDEO, "Error reading fragment shader");
        return EXIT_FAILURE;
    }
    SDL_LogDebug(SDL_LOG_CATEGORY_VIDEO, "Loaded vertex and fragment shaders");

    fclose(fpVert);
    fclose(fpFrag);

    // Create shader modules

    VkShaderModuleCreateInfo vertShadeModCreateInfo;
    vertShadeModCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    vertShadeModCreateInfo.pNext = NULL;
    vertShadeModCreateInfo.flags = 0;
    vertShadeModCreateInfo.codeSize = shaderLoaded ? vertSize : 0;
    vertShadeModCreateInfo.pCode = shaderLoaded ? (const uint32_t *)pVertCode : NULL;

    VkShaderModuleCreateInfo fragShadeModCreateInfo;
    fragShadeModCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    fragShadeModCreateInfo.pNext = NULL;
    fragShadeModCreateInfo.flags = 0;
    fragShadeModCreateInfo.codeSize = shaderLoaded ? fragSize : 0;
    fragShadeModCreateInfo.pCode = shaderLoaded ? (const uint32_t *)pFragCode : NULL;

    VkShaderModule vertShadMod, fragShadMod;
    vkCreateShaderModule(dev, &vertShadeModCreateInfo, NULL, &vertShadMod);
    SDL_LogDebug(SDL_LOG_CATEGORY_VIDEO, "created vertex shader module");
    vkCreateShaderModule(dev, &fragShadeModCreateInfo, NULL, &fragShadMod);
    SDL_LogDebug(SDL_LOG_CATEGORY_VIDEO, "created fragment shader module");

    // Fill shader stage info

    VkPipelineShaderStageCreateInfo
        vertShadStageCreateInfo,
        fragShadStageCreateInfo,
        shadStageCreateInfos[2];

    vertShadStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertShadStageCreateInfo.pNext = NULL;
    vertShadStageCreateInfo.flags = 0;
    vertShadStageCreateInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertShadStageCreateInfo.module = vertShadMod;
    char vertEntry[VK_MAX_EXTENSION_NAME_SIZE];
    strcpy(vertEntry, "main");
    vertShadStageCreateInfo.pName = vertEntry;
    vertShadStageCreateInfo.pSpecializationInfo = NULL;

    fragShadStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragShadStageCreateInfo.pNext = NULL;
    fragShadStageCreateInfo.flags = 0;
    fragShadStageCreateInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragShadStageCreateInfo.module = fragShadMod;
    char fragEntry[VK_MAX_EXTENSION_NAME_SIZE];
    strcpy(fragEntry, "main");
    fragShadStageCreateInfo.pName = fragEntry;
    fragShadStageCreateInfo.pSpecializationInfo = NULL;

    shadStageCreateInfos[0] = vertShadStageCreateInfo;
    shadStageCreateInfos[1] = fragShadStageCreateInfo;

    // Vertex input state info
    VkPipelineVertexInputStateCreateInfo vertInputInfo;
    vertInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertInputInfo.pNext = NULL;
    vertInputInfo.flags = 0;
    vertInputInfo.vertexBindingDescriptionCount = 0;
    vertInputInfo.pVertexBindingDescriptions = NULL;
    vertInputInfo.vertexAttributeDescriptionCount = 0;
    vertInputInfo.pVertexAttributeDescriptions = NULL;

    // Input assembly state info
    VkPipelineInputAssemblyStateCreateInfo inputAsmInfo;
    inputAsmInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAsmInfo.pNext = NULL;
    inputAsmInfo.flags = 0;
    inputAsmInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAsmInfo.primitiveRestartEnable = VK_FALSE;

    // Fill viewport
    VkViewport viewport;
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float)swapCreateInfo.imageExtent.width;
    viewport.height = (float)swapCreateInfo.imageExtent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    // Scissor
    VkRect2D scissor;
    VkOffset2D sciOffset;
    sciOffset.x = 0;
    sciOffset.y = 0;
    scissor.offset = sciOffset;
    scissor.extent = swapCreateInfo.imageExtent;

    // Viewport state info
    VkPipelineViewportStateCreateInfo vwpStateCreateInfo;
    vwpStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    vwpStateCreateInfo.pNext = NULL;
    vwpStateCreateInfo.flags = 0;
    vwpStateCreateInfo.viewportCount = 1;
    vwpStateCreateInfo.pViewports = &viewport;
    vwpStateCreateInfo.scissorCount = 1;
    vwpStateCreateInfo.pScissors = &scissor;

    // Rasterizer state info
    VkPipelineRasterizationStateCreateInfo rastCreateInfo;
    rastCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rastCreateInfo.pNext = NULL;
    rastCreateInfo.flags = 0;
    rastCreateInfo.depthClampEnable = VK_FALSE;
    rastCreateInfo.rasterizerDiscardEnable = VK_FALSE;
    rastCreateInfo.polygonMode = VK_POLYGON_MODE_FILL;
    rastCreateInfo.cullMode = VK_CULL_MODE_BACK_BIT;
    rastCreateInfo.frontFace = VK_FRONT_FACE_CLOCKWISE;
    rastCreateInfo.depthBiasEnable = VK_FALSE;
    rastCreateInfo.depthBiasConstantFactor = 0.0f;
    rastCreateInfo.depthBiasClamp = 0.0f;
    rastCreateInfo.depthBiasSlopeFactor = 0.0f;
    rastCreateInfo.lineWidth = 1.0f;

    // Multisampler
    VkPipelineMultisampleStateCreateInfo mulSamCreateInfo;
    mulSamCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    mulSamCreateInfo.pNext = NULL;
    mulSamCreateInfo.flags = 0;
    mulSamCreateInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    mulSamCreateInfo.sampleShadingEnable = VK_FALSE;
    mulSamCreateInfo.minSampleShading = 1.0f;
    mulSamCreateInfo.pSampleMask = NULL;
    mulSamCreateInfo.alphaToCoverageEnable = VK_FALSE;
    mulSamCreateInfo.alphaToOneEnable = VK_FALSE;

    // Color blend attachment
    VkPipelineColorBlendAttachmentState colorBlendAttach;
    colorBlendAttach.blendEnable = VK_FALSE;
    colorBlendAttach.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
    colorBlendAttach.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
    colorBlendAttach.colorBlendOp = VK_BLEND_OP_ADD;
    colorBlendAttach.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    colorBlendAttach.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    colorBlendAttach.alphaBlendOp = VK_BLEND_OP_ADD;
    colorBlendAttach.colorWriteMask =
        VK_COLOR_COMPONENT_R_BIT |
        VK_COLOR_COMPONENT_G_BIT |
        VK_COLOR_COMPONENT_B_BIT |
        VK_COLOR_COMPONENT_A_BIT;

    // Color blend state info
    VkPipelineColorBlendStateCreateInfo colorBlendCreateInfo;
    colorBlendCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlendCreateInfo.pNext = NULL;
    colorBlendCreateInfo.flags = 0;
    colorBlendCreateInfo.logicOpEnable = VK_FALSE;
    colorBlendCreateInfo.logicOp = VK_LOGIC_OP_COPY;
    colorBlendCreateInfo.attachmentCount = 1;
    colorBlendCreateInfo.pAttachments = &colorBlendAttach;
    for (uint32_t i = 0; i < 4; i++) {
        colorBlendCreateInfo.blendConstants[i] = 0.0f;
    }

    // Create pipeline layout
    VkPipelineLayoutCreateInfo pipeLayoutCreateInfo;
    pipeLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipeLayoutCreateInfo.pNext = NULL;
    pipeLayoutCreateInfo.flags = 0;
    pipeLayoutCreateInfo.setLayoutCount = 0;
    pipeLayoutCreateInfo.pSetLayouts = NULL;
    pipeLayoutCreateInfo.pushConstantRangeCount = 0;
    pipeLayoutCreateInfo.pPushConstantRanges = NULL;

    VkPipelineLayout pipeLayout;
    vkCreatePipelineLayout(dev, &pipeLayoutCreateInfo, NULL, &pipeLayout);
    SDL_LogDebug(SDL_LOG_CATEGORY_VIDEO, "Created pipeline layout");

    // Create pipeline
    VkGraphicsPipelineCreateInfo pipeCreateInfo;
    pipeCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipeCreateInfo.pNext = NULL;
    pipeCreateInfo.flags = 0;
    pipeCreateInfo.stageCount = 2;
    pipeCreateInfo.pStages = shadStageCreateInfos;
    pipeCreateInfo.pVertexInputState = &vertInputInfo;
    pipeCreateInfo.pInputAssemblyState = &inputAsmInfo;
    pipeCreateInfo.pTessellationState = NULL;
    pipeCreateInfo.pViewportState = &vwpStateCreateInfo;
    pipeCreateInfo.pRasterizationState = &rastCreateInfo;
    pipeCreateInfo.pMultisampleState = &mulSamCreateInfo;
    pipeCreateInfo.pDepthStencilState = NULL;
    pipeCreateInfo.pColorBlendState = &colorBlendCreateInfo;
    pipeCreateInfo.pDynamicState = NULL;

    pipeCreateInfo.layout = pipeLayout;
    pipeCreateInfo.renderPass = rendp;
    pipeCreateInfo.subpass = 0;
    pipeCreateInfo.basePipelineHandle = VK_NULL_HANDLE;
    pipeCreateInfo.basePipelineIndex = -1;

    VkPipeline pipe;
    vkCreateGraphicsPipelines(dev, VK_NULL_HANDLE, 1, &pipeCreateInfo, NULL, &pipe);
    SDL_LogDebug(SDL_LOG_CATEGORY_VIDEO, "Graphics pipeline created");

    // Destroy shader module
    vkDestroyShaderModule(dev, fragShadMod, NULL);
    vkDestroyShaderModule(dev, vertShadMod, NULL);
    free(pFragCode);
    free(pVertCode);

    // Create framebuffer
    VkFramebufferCreateInfo frameBufferCreateInfos[swapImageCount];
    VkFramebuffer frameBuffers[swapImageCount];
    VkImageView imageAttachs[swapImageCount];

    for (uint32_t i = 0; i < swapImageCount; i++) {
        imageAttachs[i] = imageViews[i];
        frameBufferCreateInfos[i].sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        frameBufferCreateInfos[i].pNext = NULL;
        frameBufferCreateInfos[i].flags = 0;
        frameBufferCreateInfos[i].renderPass = rendp;
        frameBufferCreateInfos[i].attachmentCount = 1;
        frameBufferCreateInfos[i].pAttachments =
            &(imageAttachs[i]);
        frameBufferCreateInfos[i].width = swapCreateInfo.imageExtent.width;
        frameBufferCreateInfos[i].height = swapCreateInfo.imageExtent.height;
        frameBufferCreateInfos[i].layers = 1;

        vkCreateFramebuffer(dev, &(frameBufferCreateInfos[i]),
                            NULL, &(frameBuffers[i]));
        SDL_LogDebug(SDL_LOG_CATEGORY_VIDEO, "Framebuffer %d created", i);
    }

    // Create command pool
    VkCommandPoolCreateInfo cmdPoolCreateInfo;
    cmdPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    cmdPoolCreateInfo.pNext = NULL;
    cmdPoolCreateInfo.flags = 0;
    cmdPoolCreateInfo.queueFamilyIndex = qfBestIndex;

    VkCommandPool cmdPool;
    vkCreateCommandPool(dev, &cmdPoolCreateInfo, NULL, &cmdPool);
    SDL_LogDebug(SDL_LOG_CATEGORY_VIDEO, "Command pool created");

    // Allocate command buffers
    VkCommandBufferAllocateInfo cmdBuffAllocInfo;
    cmdBuffAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    cmdBuffAllocInfo.pNext = NULL;
    cmdBuffAllocInfo.commandPool = cmdPool;
    cmdBuffAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    cmdBuffAllocInfo.commandBufferCount = swapImageCount;

    VkCommandBuffer cmdBuffers[swapImageCount];
    vkAllocateCommandBuffers(dev, &cmdBuffAllocInfo, cmdBuffers);
    SDL_LogDebug(SDL_LOG_CATEGORY_VIDEO, "Allocated command buffers");

    VkCommandBufferBeginInfo cmdBuffBeginInfos[swapImageCount];
    VkRenderPassBeginInfo rendpBeginInfos[swapImageCount];
    VkRect2D rendpArea;

    rendpArea.offset.x = 0;
    rendpArea.offset.y = 0;
    rendpArea.extent = swapCreateInfo.imageExtent;
    VkClearValue clearVal = {{{0.0f, 0.2f, 0.8f, 0.0f}}};

    for (uint32_t i = 0; i < swapImageCount; i++) {
        cmdBuffBeginInfos[i].sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        cmdBuffBeginInfos[i].pNext = NULL;
        cmdBuffBeginInfos[i].flags = 0;
        cmdBuffBeginInfos[i].pInheritanceInfo = NULL;

        rendpBeginInfos[i].sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        rendpBeginInfos[i].pNext = NULL;
        rendpBeginInfos[i].renderPass = rendp;
        rendpBeginInfos[i].framebuffer = frameBuffers[i];
        rendpBeginInfos[i].renderArea = rendpArea;
        rendpBeginInfos[i].clearValueCount = 1;
        rendpBeginInfos[i].pClearValues = &clearVal;

        vkBeginCommandBuffer(cmdBuffers[i], &cmdBuffBeginInfos[i]);
        vkCmdBeginRenderPass(cmdBuffers[i], &(rendpBeginInfos[i]), VK_SUBPASS_CONTENTS_INLINE);
        vkCmdBindPipeline(cmdBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipe);
        vkCmdDraw(cmdBuffers[i], 3, 1, 0, 0);
        vkCmdEndRenderPass(cmdBuffers[i]);
        vkEndCommandBuffer(cmdBuffers[i]);
        SDL_LogDebug(SDL_LOG_CATEGORY_VIDEO, "Command buffer drawing recorded");
    }

    // Semaphores and fences creation
    const uint32_t maxFrames = 2;
    VkSemaphore sempsImgAvl[maxFrames];
    VkSemaphore sempsRendFin[maxFrames];
    VkFence fens[maxFrames];

    VkSemaphoreCreateInfo sempCreateInfo;
    sempCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    sempCreateInfo.pNext = NULL;
    sempCreateInfo.flags = 0;

    VkFenceCreateInfo fenCreateInfo;
    fenCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenCreateInfo.pNext = NULL;
    fenCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (uint32_t i = 0; i < maxFrames; i++) {
        vkCreateSemaphore(dev, &sempCreateInfo, NULL, &(sempsImgAvl[i]));
        vkCreateSemaphore(dev, &sempCreateInfo, NULL, &(sempsRendFin[i]));
        vkCreateFence(dev, &fenCreateInfo, NULL, &(fens[i]));
    }
    SDL_LogDebug(SDL_LOG_CATEGORY_VIDEO, "Created semaphores and fences");

    uint32_t currFrame = 0;
    VkFence fensImg[swapImageCount];
    for (uint32_t i = 0; i < swapImageCount; i++) {
        fensImg[i] = VK_NULL_HANDLE;
    }

    // MAIN PRESENTATION PART
    int running = 1;
    while (running) {
        SDL_Event event;
        while (SDL_PollEvent(&event))
            if (event.type == SDL_QUIT) {
                running = 0;
                break;
            }

        vkWaitForFences(dev, 1, &(fens[currFrame]), VK_TRUE, UINT64_MAX);

        uint32_t imgIndex = 0;
        vkAcquireNextImageKHR(dev, swap, UINT64_MAX, sempsImgAvl[currFrame], VK_NULL_HANDLE, &imgIndex);

        if (fensImg[imgIndex] != VK_NULL_HANDLE) {
            vkWaitForFences(dev, 1, &(fensImg[imgIndex]), VK_TRUE, UINT64_MAX);
        }

        fensImg[imgIndex] = fens[currFrame];

        VkSubmitInfo subInfo;
        subInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        subInfo.pNext = NULL;

        VkSemaphore sempsWait[1];
        sempsWait[0] = sempsImgAvl[currFrame];
        VkPipelineStageFlags waitStages[1];
        waitStages[0] = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

        subInfo.waitSemaphoreCount = 1;
        subInfo.pWaitSemaphores = &(sempsWait[0]);
        subInfo.pWaitDstStageMask = &(waitStages[0]);
        subInfo.commandBufferCount = 1;
        subInfo.pCommandBuffers = &(cmdBuffers[imgIndex]);

        VkSemaphore sempsSig[1];
        sempsSig[0] = sempsRendFin[currFrame];

        subInfo.signalSemaphoreCount = 1;
        subInfo.pSignalSemaphores = &(sempsSig[0]);

        vkResetFences(dev, 1, &(fens[currFrame]));
        vkQueueSubmit(qGraph, 1, &subInfo, fens[currFrame]);

        // Present
        VkPresentInfoKHR presInfo;
        presInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        presInfo.pNext = NULL;
        presInfo.waitSemaphoreCount = 1;
        presInfo.pWaitSemaphores = &(sempsSig[0]);

        VkSwapchainKHR swaps[1];
        swaps[0] = swap;
        presInfo.swapchainCount = 1;
        presInfo.pSwapchains = &(swaps[0]);
        presInfo.pImageIndices = &imgIndex;
        presInfo.pResults = NULL;

        vkQueuePresentKHR(qPres, &presInfo);

        currFrame = (currFrame + 1) % maxFrames;
    }

    vkDeviceWaitIdle(dev);
    SDL_LogDebug(SDL_LOG_CATEGORY_VIDEO, "command buffers finished");

    // Cleanup
    vkFreeCommandBuffers(dev, cmdPool, swapImageCount, cmdBuffers);
    for (uint32_t i = 0; i < maxFrames; i++) {
        vkDestroySemaphore(dev, sempsRendFin[i], NULL);
        vkDestroySemaphore(dev, sempsImgAvl[i], NULL);
        vkDestroyFence(dev, fens[i], NULL);
    }
    vkDestroyCommandPool(dev, cmdPool, NULL);

    for (uint32_t i = 0; i < swapImageCount; i++) {
        vkDestroyFramebuffer(dev, frameBuffers[i], NULL);
    }

    vkDestroyPipeline(dev, pipe, NULL);
    vkDestroyPipelineLayout(dev, pipeLayout, NULL);
    vkDestroyRenderPass(dev, rendp, NULL);

    for (uint32_t i = 0; i < swapImageCount; i++) {
        vkDestroyImageView(dev, imageViews[i], NULL);
    }

    vkDestroySwapchainKHR(dev, swap, NULL);
    vkDestroySurfaceKHR(vkInst, surf, NULL);
    SDL_DestroyWindow(window);

    vkDestroyDevice(dev, NULL);
    deleteInstance(&vkInst);

    SDL_Vulkan_UnloadLibrary();
    SDL_Quit();

    return EXIT_SUCCESS;
}
