#include "../core.h"
#include <stdlib.h>
#include <time.h>
#include <string.h>

#define MU_DEBUG
#include <MemoryUtilities/MemoryUtilities.h>

void* avAllocate_(uint typeSize, uint count, AV_LOCATION_ARGS, const char* errorMsg) {
	void* data = fsAllocate((uint64)typeSize * (uint64)count);
	if (data == 0) {
		avAssert_(AV_MEMORY_ERROR, AV_SUCCESS, line, file, func, fstream, errorMsg);
		return NULL;
	}
	memset(data, 0, (uint64)typeSize * (uint64)count);
	return data;
}

void avFree(void* data) {
	fsFree(data);
}

#pragma region AV_LOGGING
AvLogLevel AV_LOG_LEVEL = AV_LOG_LEVEL_DEFAULT;
uint AV_LOG_LINE = AV_LOG_LINE_DEFAULT;
uint AV_LOG_FILE = AV_LOG_FILE_DEFAULT;
uint AV_LOG_FUNC = AV_LOG_FUNC_DEFAULT;
uint AV_LOG_PROJECT = AV_LOG_PROJECT_DEFAULT;
uint AV_LOG_TIME = AV_LOG_TIME_DEFAULT;
uint AV_LOG_TYPE = AV_LOG_TYPE_DEFAULT;
uint AV_LOG_ERROR = AV_LOG_ERROR_DEFAULT;
AvAssertLevel AV_ASSERT_LEVEL = AV_ASSERT_LEVEL_DEFAULT;

char AV_LOG_PROJECT_NAME[64] = "PROJECT_NAME_NOT_SPECIFIED";
uint AV_LOG_PROJECT_VERSION = 0;

const AvLogSettings avLogSettingsDefault = {
	.level = AV_LOG_LEVEL_DEFAULT,
	.printLine = AV_LOG_LINE_DEFAULT,
	.printFile = AV_LOG_FILE_DEFAULT,
	.printFunc = AV_LOG_FUNC_DEFAULT,
	.printProject = AV_LOG_PROJECT_DEFAULT,
	.printTime = AV_LOG_TIME_DEFAULT,
	.printType = AV_LOG_TYPE_DEFAULT,
	.printError = AV_LOG_ERROR_DEFAULT,
	.assertLevel = AV_ASSERT_LEVEL_DEFAULT,
};

#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_RESET   "\x1b[0m"

void setLogLevel(AvLogSettings settings) {
	AV_LOG_LEVEL = settings.level;
	AV_LOG_LINE = settings.printLine;
	AV_LOG_FILE = settings.printFile;
	AV_LOG_FUNC = settings.printFunc;
	AV_LOG_PROJECT = settings.printProject;
	AV_LOG_TIME = settings.printTime;
	AV_LOG_TYPE = settings.printType;
	AV_LOG_ERROR = settings.printError;
	AV_ASSERT_LEVEL = settings.assertLevel;
}

void setProjectDetails(const char* projectName, uint version) {

	strcpy_s((void*)AV_LOG_PROJECT_NAME, 63, projectName);
	AV_LOG_PROJECT_NAME[63] = '\0';

	AV_LOG_PROJECT_VERSION = version;
}

void printTags(AvResult result, AV_LOCATION_ARGS, const char* msg) {
	//message
	const char* message;

	switch (result) {
	case AV_SUCCESS:
		message = "success";
		break;
	case AV_DEBUG:
		message = "debug";
		break;
	case AV_INFO:
		message = "info";
		break;
	case AV_WARNING:
		message = "warning";
		break;
	case AV_ERROR:
		message = "error";
		break;

	case AV_OUT_OF_BOUNDS:
		message = "out of bounds";
		break;
	case AV_MEMORY_ERROR:
		message = "memory error";
		break;
	case AV_CREATION_ERROR:
		message = "creation error";
		break;
	case AV_NO_SUPPORT:
		message = "no support";
		break;
	case AV_INVALID_ARGUMENTS:
		message = "invalid arguments";
		break;
	case AV_TIMEOUT:
		message = "timeout";
		break;
	case AV_VALIDATION_NOT_PRESEND:
		message = "validation not present";
		break;
	default:
		message = "unknown";
		break;
	}

	if (AV_LOG_TIME) {
		//time 
		time_t rawtime;
		struct tm* timeinfo;
		time(&rawtime);
		timeinfo = localtime(&rawtime);
		char time[80];
		strftime(time, 80, "%T", timeinfo);
		fprintf(fstream, "[%s]", time);
	}
	if (AV_LOG_TYPE) {
		//level
		const char* result_level;
		if (result == AV_SUCCESS) {
			result_level = ANSI_COLOR_GREEN "SUCESS" ANSI_COLOR_RESET;
		} else if (result & AV_INFO) {
			result_level = ANSI_COLOR_RESET "INFO" ANSI_COLOR_RESET;
		} else if (result & AV_DEBUG) {
			result_level = ANSI_COLOR_BLUE "DEBUG" ANSI_COLOR_RESET;
		} else if (result & AV_WARNING) {
			result_level = ANSI_COLOR_YELLOW "WARNING" ANSI_COLOR_RESET;
		} else if (result & AV_ERROR) {
			result_level = ANSI_COLOR_RED"ERROR" ANSI_COLOR_RESET;
		} else {
			result_level = "UNKNOWN";
		}
		fprintf(fstream, "[%s]", result_level);
	}
	if (AV_LOG_PROJECT) {
		fprintf(fstream, "[%s v%i.%i]", AV_LOG_PROJECT_NAME, AV_LOG_PROJECT_VERSION >> 16, AV_LOG_PROJECT_VERSION & 0xffff);
	}
	if (AV_LOG_FUNC) {
		fprintf(fstream, "[func: %s]", func);
	}
	if (AV_LOG_LINE) {
		fprintf(fstream, "[line %lu]", line);
	}
	if (AV_LOG_FILE) {
		fprintf(fstream, "[file: %s]", file);
	}
	if (AV_LOG_ERROR) {
		fprintf(fstream, " %s", message);
	}

	if (!(strcmp(msg, "") == 0)) {
		fprintf(fstream, " -> ");
	}
}


void avLog_(AvResult result, AV_LOCATION_ARGS, const char* msg) {

	if ((uint)result < AV_LOG_LEVEL) {
		return;
	}

	printTags(result, line, file, func, fstream, msg);

	fprintf(fstream, "%s\n", msg);

}

void avAssert_(AvResult result, AvResult valid, AV_LOCATION_ARGS, const char* msg) {
	if (result == valid && AV_ASSERT_LEVEL != AV_ASSERT_LEVEL_ALL) {
		return;
	}

	printTags(result, line, file, func, fstream, msg);
	fprintf(fstream, "assert -> ");
	fprintf(fstream, "%s\n", msg);

	if (result >= AV_ASSERT_LEVEL && result) {
		abort();
	}
}

#pragma endregion




bool checkValidationLayerSupport() {
	uint32_t layerCount;
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
	if(messageType & VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT){
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

	return indices;
}

bool isQueueFamilyIndicesComplete(QueueFamilyIndices indices) {
	return indices.graphicsFamilyPresent;//&& indices.presentFamilyPresent;
}
