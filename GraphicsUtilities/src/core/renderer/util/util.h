#pragma once
#include "../renderer.h"
#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>
// validation
extern const char* const validationLayers[];
extern const uint validationLayerCount;
bool checkValidationLayerSupport();
VkResult createDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger);
void destroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator);
void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT* createInfo);

// physical device
typedef struct QueueFamilyIndices {
	uint32 graphicsFamily;
	bool graphicsFamilyPresent;

	uint32 presentFamily;
	bool presentFamilyPresent;
}QueueFamilyIndices;
bool isQueueFamilyIndicesComplete(QueueFamilyIndices indices);
QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device, AvInstance instance);

bool isDeviceSuitable(VkPhysicalDevice device);
uint scoreDevice(VkPhysicalDevice device);