#include "util.h"

bool checkValidationLayerSupport() {
	uint32 layerCount;
	vkEnumerateInstanceLayerProperties(&layerCount, NULL);

	VkLayerProperties* availableLayers = avAllocate(sizeof(VkLayerProperties), layerCount, "could not allocate memory for validation layer enumeration!");
	vkEnumerateInstanceLayerProperties(&layerCount, availableLayers);

	for (uint i = 0; i < validationLayerCount; i++) {
		bool layerFound = 0;
		for (uint j = 0; j < layerCount; j++) {

			if (strcmp(validationLayers[i], availableLayers[j].layerName) == 0) {
				layerFound = true;
				break;
			}

		}
		if (!layerFound) {
			avFree(availableLayers);
			return false;
		}
	}
	avFree(availableLayers);
	return true;
}

static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
	VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
	VkDebugUtilsMessageTypeFlagsEXT messageType,
	const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
	void* pUserData) {

	switch (AV_LOG_LEVEL) {
	case AV_LOG_LEVEL_ALL:
		break;
	case AV_LOG_LEVEL_DEBUG:
		if (messageSeverity < VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT) {
			return VK_FALSE;
		}
		break;
	case AV_LOG_LEVEL_INFO:
		if (messageSeverity < VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT) {
			return VK_FALSE;
		}
		break;
	case AV_LOG_LEVEL_WARNING:
		if (messageSeverity < VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
			return VK_FALSE;
		}
		break;
	case AV_LOG_LEVEL_ERROR:
		if (messageSeverity < VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) {
			return VK_FALSE;
		}
		break;
	case AV_LOG_LEVEL_NONE:
		return VK_FALSE;
	}

	fprintf(stdout, ANSI_COLOR_RESET"["ANSI_COLOR_CYAN"vulkan"ANSI_COLOR_RESET"]");

	const char* type;
	if (messageType & VK_DEBUG_UTILS_MESSAGE_TYPE_DEVICE_ADDRESS_BINDING_BIT_EXT) {
		fprintf(stdout, ANSI_COLOR_RESET"["ANSI_COLOR_YELLOW"address"ANSI_COLOR_RESET"]");
	}
	if (messageType & VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT) {
		fprintf(stdout, ANSI_COLOR_RESET"["ANSI_COLOR_CYAN"general"ANSI_COLOR_RESET"]");
	}
	if (messageType & VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT) {
		fprintf(stdout, ANSI_COLOR_RESET"["ANSI_COLOR_RED"validation"ANSI_COLOR_RESET"]");
	}
	if (messageType & VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT) {
		fprintf(stdout, ANSI_COLOR_RESET"["ANSI_COLOR_BLUE"performance"ANSI_COLOR_RESET"]");
	}
	if (messageSeverity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
		fprintf(stdout, ANSI_COLOR_RESET"["ANSI_COLOR_YELLOW"WARNING"ANSI_COLOR_RESET"]");
	}
	if (messageSeverity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) {
		fprintf(stdout, ANSI_COLOR_RESET"["ANSI_COLOR_RED"ERROR"ANSI_COLOR_RESET"]");
	}
	fprintf(stdout, " -> %s\n", pCallbackData->pMessage);

	return VK_FALSE;
}

VkResult createDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger) {
	PFN_vkCreateDebugUtilsMessengerEXT func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
	if (func != NULL) {
		return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
	} else {
		return VK_ERROR_EXTENSION_NOT_PRESENT;
	}
}

void destroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator) {
	PFN_vkDestroyDebugUtilsMessengerEXT func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
	if (func != NULL) {
		func(instance, debugMessenger, pAllocator);
	}
}

void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT* createInfo) {
	createInfo->flags = 0;
	createInfo->sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	createInfo->messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	createInfo->messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	createInfo->pfnUserCallback = debugCallback;
	createInfo->pUserData = 0;
	createInfo->pNext = 0;
}

bool isDeviceSuitable(VkPhysicalDevice device) {
	VkPhysicalDeviceProperties deviceProperties;
	vkGetPhysicalDeviceProperties(device, &deviceProperties);

	VkPhysicalDeviceFeatures deviceFeatures;
	vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

	findQueueFamilies(device, nullptr);

	//vkGetPhysicalDeviceSurfaceSupportKHR()
	return true;
}

uint scoreDevice(VkPhysicalDevice device) {
	VkPhysicalDeviceProperties deviceProperties;
	vkGetPhysicalDeviceProperties(device, &deviceProperties);

	VkPhysicalDeviceFeatures deviceFeatures;
	vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

	if (!isDeviceSuitable(device)) {
		return 0;
	}

	uint score = 1;

	if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU ||
		deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_CPU) {
		score++;
	}

	return score;
}

QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device, AvInstance instance) {
	QueueFamilyIndices indices = { 0 };

	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

	VkQueueFamilyProperties* queueFamilies = avAllocate(sizeof(VkQueueFamilyProperties), queueFamilyCount, "allocating for queue family enumeration");
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies);

	for (uint i = 0; i < queueFamilyCount; i++) {
		VkQueueFamilyProperties queueFamily = queueFamilies[i];
		if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
			indices.graphicsFamily = i;
			indices.graphicsFamilyPresent = true;

			if (isQueueFamilyIndicesComplete(indices)) {
				break;
			}

		}
	}

	avFree(queueFamilies);

	return indices;
}

bool isQueueFamilyIndicesComplete(QueueFamilyIndices indices) {
	return indices.graphicsFamilyPresent;//&& indices.presentFamilyPresent;
}
