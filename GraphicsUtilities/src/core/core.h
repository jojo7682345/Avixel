#pragma once
#include <GraphicsUtilities.h>
#include "util/util.h"
#include "renderer/renderer.h"
#include "logging/logging.h"
#include "display/display.h"



typedef struct AvInstance_T {
	DisplaySurface displaySurface;
	Window window;
}AvInstance_T;



//static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData);
