#pragma once
#include <GraphicsUtilities.h>
#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>
#include "util/util.h"
#include "renderer/renderer.h"



typedef struct AvInstance_T {
	Renderer;
}AvInstance_T;

typedef struct AvWindow_T {
	AvInstance instance;
	Window;
}AvWindow_T;

static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData);
