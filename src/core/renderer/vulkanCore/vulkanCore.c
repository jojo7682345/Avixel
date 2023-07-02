#include "../renderer.h"
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <string.h>
#include <stdio.h>

#define MAX_FRAMES_IN_FLIGHT  2

const char* const requiredDefaultExtensions[] = {
	VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
};
const uint requiredDefaultExtensionCount = sizeof(requiredDefaultExtensions) / sizeof(const char*);

const char* const deviceExtensions[] = {
	VK_KHR_SWAPCHAIN_EXTENSION_NAME,
};
const uint deviceExtensionCount = sizeof(deviceExtensions) / sizeof(const char*);

const char* const validationLayers[] = {
	"VK_LAYER_KHRONOS_validation",
};
const uint validationLayerCount = sizeof(validationLayers) / sizeof(const char*);


typedef struct RenderInstance_T {
	AvInstance globalInstance;
	VkInstance instance;
	VkDebugUtilsMessengerEXT* debugMessenger;
}RenderInstance_T;

typedef struct QueueFamilyIndices {
	uint32 graphicsFamily;
	bool graphicsFamilyPresent;

	uint32 presentFamily;
	bool presentFamilyPresent;
}QueueFamilyIndices;

typedef struct SwapChainSupportDetails {
	VkSurfaceCapabilitiesKHR capabilities;
	uint formatCount;
	VkSurfaceFormatKHR* formats;
	uint presetnModeCount;
	VkPresentModeKHR* presentModes;
}SwapChainSupportDetails;

typedef struct RenderDevice_T {
	RenderInstance instance;

	VkPhysicalDevice physicalDevice;
	VkDevice device;

	QueueFamilyIndices queueFamilyIndices;
	VkQueue graphicsQueue;
	VkQueue presentQueue;
} RenderDevice_T;

typedef struct Frame {
	VkFramebuffer framebuffer;
	VkImage image;
	VkImageView imageView;

	VkCommandBuffer commandBuffer;
	VkCommandPool* commandPool;

	VkSemaphore imageAvailable;
	VkSemaphore renderFinished;
	VkFence inFlight;

	VkFormat* format;
	VkExtent2D* extent;
}Frame;

typedef struct Window_T {
	AvInstance instance;

	DisplaySurface displaySurface;
	GLFWwindow* window;

	VkSurfaceKHR surface;

	VkSwapchainKHR swapchain;

	VkExtent2D frameExtent;
	VkFormat frameFormat;

	VkCommandPool commandPool;

	uint frameCount;
	Frame* frames;
} Window_T;

typedef struct DisplaySurface_T {
	uint width;
	uint height;
	DisplayType type;
} DisplaySurface_T;

AvResult displaySurfaceInit(AvInstance instance) {

	glfwInit();

	instance->displaySurface = avAllocate(sizeof(DisplaySurface_T), 1, "allocating displaysurface handle");

	GLFWmonitor* monitor = glfwGetPrimaryMonitor();

	if (monitor == NULL) {
		return AV_NO_SUPPORT;
	}

	const GLFWvidmode* mode = glfwGetVideoMode(monitor);

	instance->displaySurface->width = mode->width;
	instance->displaySurface->height = mode->height;
	instance->displaySurface->type = DISPLAY_TYPE_MONITOR;

	char monitorSize[32];
	sprintf(monitorSize, "display surface size %ix%i", mode->width, mode->height);
	avLog(AV_INFO, monitorSize);

	avAssert(0, AV_SUCCESS, "initialized display surface");

	return AV_SUCCESS;
}

uint displaySurfaceGetWidth(AvInstance instance) {
	return instance->displaySurface->width;
}

uint displaySurfaceGetHeight(AvInstance instance) {
	return instance->displaySurface->height;
}

DisplayType displaySurfaceGetType(AvInstance instance) {
	return instance->displaySurface->type;
}

void displaySurfaceDeinit(AvInstance instance) {
	glfwTerminate();

	avFree(instance->displaySurface);

	avAssert(0, AV_SUCCESS, "deinitialized display surface");
}

const char** displaySurfaceEnumerateExtensions(AvInstance instance, uint* count) {
	return glfwGetRequiredInstanceExtensions(count);
}

AvResult displaySurfaceCreateWindow(AvInstance instance, WindowCreateInfo windowCreateInfo, WindowProperties* windowProperties, Window* window) {

	if (windowCreateInfo.properties.size.width > instance->displaySurface->width) {
		windowCreateInfo.properties.size.width = instance->displaySurface->width;
	}
	if (windowCreateInfo.properties.size.height > instance->displaySurface->height) {
		windowCreateInfo.properties.size.height = instance->displaySurface->height;
	}

	*window = avAllocate(sizeof(Window_T), 1, "allocating window handle");
	(*window)->instance = instance;
	(*window)->displaySurface = instance->displaySurface;


	GLFWmonitor* fullscreen = windowCreateInfo.properties.fullSurface ? glfwGetPrimaryMonitor() : NULL;

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, windowCreateInfo.properties.resizable ? GLFW_TRUE : GLFW_FALSE);
	(*window)->window = glfwCreateWindow(windowCreateInfo.properties.size.width, windowCreateInfo.properties.size.height, windowCreateInfo.properties.title, fullscreen, NULL);
	if((*window)->window==NULL){
		avAssert(AV_CREATION_ERROR,0, "Failed to create the window");
	}
	avAssert(0, 0, "created window");

	if (windowCreateInfo.properties.size.x != -1 && windowCreateInfo.properties.size.y != -1) {
		glfwSetWindowPos((*window)->window, windowCreateInfo.properties.size.x, windowCreateInfo.properties.size.y);
	} else if ((windowCreateInfo.properties.size.x == -1 && windowCreateInfo.properties.size.y != -1) || (windowCreateInfo.properties.size.x != -1 && windowCreateInfo.properties.size.y == -1)) {
		avAssert(AV_UNUSUAL_ARGUMENTS, AV_SUCCESS, "single axis specified, should be both or none");
	}

	glfwShowWindow((*window)->window);

	glfwSetWindowCloseCallback((*window)->window, windowCreateInfo.onWindowDisconnect);
	if (windowCreateInfo.properties.resizable) {
		if (!windowCreateInfo.onWindowResize) {
			avAssert(AV_UNSPECIFIED_CALLBACK, AV_SUCCESS, "window is resizable, but no resize callback is specified");
		} else {
			glfwSetWindowSizeCallback((*window)->window, windowCreateInfo.onWindowResize);
		}
	}

	uint width = 0, height = 0, x = 0, y = 0;
	glfwGetWindowSize((*window)->window, &width, &height);
	glfwGetWindowPos((*window)->window, &x, &y);

	if (windowProperties) {
		windowProperties->fullSurface = windowCreateInfo.properties.fullSurface;
		windowProperties->resizable = windowCreateInfo.properties.resizable;
		windowProperties->title = windowCreateInfo.properties.title;
		windowProperties->size.x = x;
		windowProperties->size.y = y;
		windowProperties->size.width = width;
		windowProperties->size.height = height;
	}

	VkResult result = glfwCreateWindowSurface(instance->renderInstance->instance, (*window)->window, NULL, &((*window)->surface));
	if (result) {
		avAssert(AV_NO_SUPPORT, AV_SUCCESS, "failed to create window surface");
	}

	avAssert(0, 0, "created window surface");
	return AV_SUCCESS;
}

void displaySurfaceDestroyWindow(AvInstance instance, Window window) {
	vkDestroySurfaceKHR(instance->renderInstance->instance, window->surface, nullptr);
	avFree(window);
}

void enumerateRequiredExtensions(uint* requiredExtensionCount, const char*** requiredExtensions, AvInstance instance) {
	uint glfwExtensionCount = 0;
	const char** glfwExtenstions;
	glfwExtenstions = displaySurfaceEnumerateExtensions(instance, &glfwExtensionCount);

	*requiredExtensionCount = glfwExtensionCount + requiredDefaultExtensionCount;

	if (requiredExtensions == NULL) {
		return;
	}

	uint index = 0;
	for (uint i = 0; i < requiredDefaultExtensionCount; i++) {
		(*requiredExtensions)[index++] = requiredDefaultExtensions[i];
	}
	for (uint i = 0; i < glfwExtensionCount; i++) {
		(*requiredExtensions)[index++] = glfwExtenstions[i];
	}

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

static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
	VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
	VkDebugUtilsMessageTypeFlagsEXT messageType,
	const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
	void* pUserData) {

	AvLogLevel level = { 0 };
	if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT) {
		level = AV_LOG_LEVEL_ALL;
	}
	if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT) {
		level = AV_LOG_LEVEL_INFO;
	}
	if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
		level = AV_LOG_LEVEL_WARNING;
	}
	if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) {
		level = AV_LOG_LEVEL_ERROR;
	}

	ValidationMessageType type = { 0 };
	if (messageType & VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT) {
		type = VALIDATION_MESSAGE_TYPE_GENERAL;
	}
	if (messageType & VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT) {
		type = VALIDATION_MESSAGE_TYPE_VALIDATION;
	}
	if (messageType & VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT) {
		type = VALIDATION_MESSAGE_TYPE_PERFORMANCE;
	}

	logDeviceValidation("vulkan", level, type, pCallbackData->pMessage);

	return VK_FALSE;
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

void renderInstanceCreate(AvInstance instance, RenderInstanceCreateInfo info) {
	// allocate handle
	instance->renderInstance = avAllocate(sizeof(RenderInstance_T), 1, "allocating render instance");
	instance->renderInstance->globalInstance = instance;

	// instance creation
	VkApplicationInfo appInfo = { 0 };
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.apiVersion = VK_API_VERSION_1_0;
	appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.pEngineName = "AlpineValleyUIengine";
	appInfo.pApplicationName = info.projectInfo.pProjectName;
	appInfo.applicationVersion = info.projectInfo.projectVersion;
	appInfo.pNext = nullptr;

	VkInstanceCreateInfo instanceInfo = { 0 };
	instanceInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	instanceInfo.pApplicationInfo = &appInfo;
	VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo = { 0 };
	if (info.deviceValidationEnabled) {
		instanceInfo.enabledLayerCount = validationLayerCount;
		instanceInfo.ppEnabledLayerNames = validationLayers;
		populateDebugMessengerCreateInfo(&debugCreateInfo);
		instanceInfo.pNext = &debugCreateInfo;
	} else {
		instanceInfo.enabledLayerCount = 0;
	}

	uint requiredExtensionCount = 0;
	enumerateRequiredExtensions(&requiredExtensionCount, NULL, instance);
	const char** requiredExtensions = avAllocate(sizeof(const char*), requiredExtensionCount, "failed to allocate memory for exension enumeration");
	enumerateRequiredExtensions(&requiredExtensionCount, &requiredExtensions, instance);

	instanceInfo.enabledExtensionCount = requiredExtensionCount;
	instanceInfo.ppEnabledExtensionNames = requiredExtensions;

	VkResult result;
	result = vkCreateInstance(&instanceInfo, NULL, &instance->renderInstance->instance);
	if (result != VK_SUCCESS) {
		avAssert(AV_CREATION_ERROR, AV_SUCCESS, "creating the vulkan instance");
	}
	avAssert(0, AV_SUCCESS, "created the vulkan instance");
	avFree(requiredExtensions);

	// debug messenger setup
	if (info.deviceValidationEnabled) {
		VkDebugUtilsMessengerCreateInfoEXT debugInfo;
		populateDebugMessengerCreateInfo(&debugInfo);
		instance->renderInstance->debugMessenger = avAllocate(sizeof(VkDebugUtilsMessengerEXT), 1, "allocating memeory for debug messenger");
		result = createDebugUtilsMessengerEXT(instance->renderInstance->instance, &debugInfo, NULL, instance->renderInstance->debugMessenger);
		if (result != VK_SUCCESS) {
			avAssert(AV_CREATION_ERROR, AV_SUCCESS, "creating the validation logger");
		}
		avAssert(0, AV_SUCCESS, "created the validation logger");
	}
}

void renderInstanceDestroy(AvInstance instance) {

	if (instance->window) {
		displaySurfaceDestroyWindow(instance, instance->window);
		avAssert(0, AV_SUCCESS, "destroyed primary window");
	}

	if (instance->renderInstance->debugMessenger) {
		destroyDebugUtilsMessengerEXT(instance->renderInstance->instance, *(instance->renderInstance->debugMessenger), NULL);
		avFree(instance->renderInstance->debugMessenger);
		avAssert(0, AV_SUCCESS, "destroyed validation logger");
	}

	vkDestroyInstance(instance->renderInstance->instance, NULL);
	avAssert(0, AV_SUCCESS, "destroyed vulkan instance");

	avFree(instance->renderInstance);
}

bool renderInstanceCheckValidationSupport() {
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

bool isQueueFamilyIndicesComplete(QueueFamilyIndices indices) {
	return indices.graphicsFamilyPresent && indices.presentFamilyPresent;
}

QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device, Window window) {
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
		}

		uint32 presentSupport = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(device, i, window->surface, &presentSupport);

		if (presentSupport) {
			indices.presentFamily = i;
			indices.presentFamilyPresent = true;
		}

		if (isQueueFamilyIndicesComplete(indices)) {
			break;
		}
	}

	avFree(queueFamilies);

	return indices;
}


bool checkDeviceExtensionSupport(VkPhysicalDevice device) {
	uint32_t extensionCount;
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

	VkExtensionProperties* availableExtensions = avAllocate(sizeof(VkExtensionProperties), extensionCount, "enumerating device extension properties");
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions);

	for (uint i = 0; i < deviceExtensionCount; i++) {
		const char* checkExtension = deviceExtensions[i];
		bool found = false;
		for (uint j = 0; j < extensionCount; j++) {
			const char* availableExtension = availableExtensions[j].extensionName;
			if (strcmp(checkExtension, availableExtension) == 0) {
				found = true;
				break;
			}
		}
		if (!found) {
			avFree(availableExtensions);
			return false;
		}
	}
	avFree(availableExtensions);
	return true;
}

SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device, Window window) {
	SwapChainSupportDetails details = { 0 };

	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, window->surface, &details.capabilities);

	uint32_t formatCount;
	vkGetPhysicalDeviceSurfaceFormatsKHR(device, window->surface, &formatCount, nullptr);

	if (formatCount != 0) {
		details.formats = avAllocate(sizeof(VkSurfaceFormatKHR), formatCount, "allocating surface formats");
		details.formatCount = formatCount;
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, window->surface, &formatCount, details.formats);
	}

	uint32_t presentModeCount;
	vkGetPhysicalDeviceSurfacePresentModesKHR(device, window->surface, &presentModeCount, nullptr);

	if (presentModeCount != 0) {
		details.presentModes = avAllocate(sizeof(VkPresentModeKHR), presentModeCount, "allocating surface present modes");
		details.presetnModeCount = presentModeCount;
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, window->surface, &presentModeCount, details.presentModes);
	}

	return details;
}

void freeSwapchainSupport(SwapChainSupportDetails details) {
	if (details.presentModes) {
		avFree(details.presentModes);
	}
	if (details.formats) {
		avFree(details.formats);
	}
}

bool isDeviceSuitable(VkPhysicalDevice device, Window window) {
	VkPhysicalDeviceProperties deviceProperties;
	vkGetPhysicalDeviceProperties(device, &deviceProperties);

	VkPhysicalDeviceFeatures deviceFeatures;
	vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

	QueueFamilyIndices indices = findQueueFamilies(device, window);
	if (!isQueueFamilyIndicesComplete(indices)) {
		return false;
	}

	if (!checkDeviceExtensionSupport(device)) {
		return false;
	}

	SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device, window);
	if (!swapChainSupport.formatCount) {
		return false;
	}

	if (!swapChainSupport.presetnModeCount) {
		return false;
	}
	freeSwapchainSupport(swapChainSupport);

	return true;
}

uint scoreDevice(VkPhysicalDevice device, Window window) {
	VkPhysicalDeviceProperties deviceProperties;
	vkGetPhysicalDeviceProperties(device, &deviceProperties);

	VkPhysicalDeviceFeatures deviceFeatures;
	vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

	if (!isDeviceSuitable(device, window)) {
		return 0;
	}

	uint score = 1;

	if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU ||
		deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_CPU) {
		score++;
	}

	return score;
}

VkSurfaceFormatKHR chooseSwapSurfaceFormat(VkSurfaceFormatKHR* availableFormats, uint availableFormatCount) {
	for (uint i = 0; i < availableFormatCount; i++) {
		VkSurfaceFormatKHR availableFormat = availableFormats[i];
		if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
			return availableFormat;
		}
	}

	return availableFormats[0];
}

VkPresentModeKHR chooseSwapPresentMode(VkPresentModeKHR* availablePresentModes, uint availablePresentModeCount) {
	for (uint i = 0; i < availablePresentModeCount; i++) {
		VkPresentModeKHR availablePresentMode = availablePresentModes[i];
		if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
			return availablePresentMode;
		}
	}

	return VK_PRESENT_MODE_FIFO_KHR;
}

#define clamp(x, min, max) ((x) <= (min) ? (min) : ((x) >= (max) ? (max) : (x)))

VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR capabilities, Window window) {
	if (capabilities.currentExtent.width != (uint)-1) {
		return capabilities.currentExtent;
	} else {
		int width, height;
		glfwGetFramebufferSize(window->window, &width, &height);
		VkExtent2D actualExtent = {
			.width = (uint)width,
			.height = (uint)height
		};
		actualExtent.width = clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
		actualExtent.height = clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

		return actualExtent;
	}
}

void renderDeviceCreate(AvInstance instance, Window window, RenderDevice* pDevice) {
	*pDevice = avAllocate(sizeof(RenderDevice_T), 1, "allocating render instance");
	(*pDevice)->instance = instance->renderInstance;

	// physical device selection
	uint32 deviceCount = 0;
	vkEnumeratePhysicalDevices(instance->renderInstance->instance, &deviceCount, NULL);
	if (deviceCount == 0) {
		avAssert(AV_NO_SUPPORT, AV_SUCCESS, "vulkan not supported on device");
	}
	VkPhysicalDevice* devices = avAllocate(sizeof(VkPhysicalDevice), deviceCount, "failed to allocate memory for gpu enumeration");
	vkEnumeratePhysicalDevices(instance->renderInstance->instance, &deviceCount, devices);

	uint bestScore = 0;
	VkPhysicalDevice bestDevice = VK_NULL_HANDLE;
	for (uint i = 0; i < deviceCount; i++) {
		VkPhysicalDevice device = devices[i];

		VkPhysicalDeviceProperties deviceProperties;
		vkGetPhysicalDeviceProperties(device, &deviceProperties);

		const char* deviceName = deviceProperties.deviceName;
		char msg[256 + (sizeof("found device ") / sizeof(char))] = { 0 };
		sprintf(msg, "found device %s", deviceName);
		avLog(AV_INFO, msg);

		uint score = scoreDevice(device, window);
		if (score > bestScore) {
			bestDevice = device;
			bestScore = score;
		}
	}
	avFree(devices);

	(*pDevice)->physicalDevice = bestDevice;
	if ((*pDevice)->physicalDevice == VK_NULL_HANDLE) {
		avAssert(AV_NO_SUPPORT, AV_SUCCESS, "no suitable gpu found");
	} else {
		VkPhysicalDeviceProperties deviceProperties;
		vkGetPhysicalDeviceProperties((*pDevice)->physicalDevice, &deviceProperties);
		const char* deviceName = deviceProperties.deviceName;
		char msg[256 + (sizeof("selected device ") / sizeof(char))] = { 0 };
		sprintf(msg, "selected device  %s", deviceName);
		avLog(AV_INFO, msg);
	}
	avAssert(0, 0, "found physical device");

	QueueFamilyIndices indices = findQueueFamilies((*pDevice)->physicalDevice, window);
	(*pDevice)->queueFamilyIndices = indices;


	// logical device
	float queuePriority = 1.0f;
	VkDeviceQueueCreateInfo queueCreateInfos[2] = { 0 };
	queueCreateInfos[0].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	queueCreateInfos[0].queueFamilyIndex = indices.graphicsFamily;
	queueCreateInfos[0].queueCount = 1;
	queueCreateInfos[0].pQueuePriorities = &queuePriority;

	queueCreateInfos[1].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	queueCreateInfos[1].queueFamilyIndex = indices.presentFamily;
	queueCreateInfos[1].queueCount = 1;
	queueCreateInfos[1].pQueuePriorities = &queuePriority;

	uint queueCount = indices.graphicsFamily != indices.presentFamily ? 2 : 1;

	VkPhysicalDeviceFeatures deviceFeatures = { 0 };

	VkDeviceCreateInfo createInfo = { 0 };
	createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	createInfo.pQueueCreateInfos = queueCreateInfos;
	createInfo.queueCreateInfoCount = queueCount;
	createInfo.pEnabledFeatures = &deviceFeatures;

	createInfo.enabledExtensionCount = deviceExtensionCount;
	createInfo.ppEnabledExtensionNames = deviceExtensions;

	if (instance->renderInstance->debugMessenger) {
		createInfo.enabledLayerCount = validationLayerCount;
		createInfo.ppEnabledLayerNames = validationLayers;
	} else {
		createInfo.enabledLayerCount = 0;
	}
	VkResult result;
	result = vkCreateDevice((*pDevice)->physicalDevice, &createInfo, nullptr, &(*pDevice)->device);
	if (result != VK_SUCCESS) {
		avAssert(AV_CREATION_ERROR, AV_SUCCESS, "creating the vulkan logical device");
	}
	avAssert(0, 0, "created render device");

	vkGetDeviceQueue((*pDevice)->device, indices.graphicsFamily, 0, &(*pDevice)->graphicsQueue);
	vkGetDeviceQueue((*pDevice)->device, indices.presentFamily, 0, &(*pDevice)->presentQueue);

}

typedef struct FrameCreateInfo {
	VkImage* images;
	VkCommandBuffer* cmdBuffers;
} FrameCreateInfo;

void frameCreateResources(RenderDevice device, Window window, uint frameIndex, FrameCreateInfo createInfo, Frame* frame) {
	

	frame->image = createInfo.images[frameIndex];
	frame->extent = &window->frameExtent;
	frame->format = &window->frameFormat;
	frame->commandPool = &window->commandPool;
	frame->commandBuffer = createInfo.cmdBuffers[frameIndex];

	VkImageViewCreateInfo imageViewInfo = { 0 };
	imageViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	imageViewInfo.image = frame->image;
	imageViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	imageViewInfo.format = *(frame->format);
	imageViewInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
	imageViewInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
	imageViewInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
	imageViewInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
	imageViewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	imageViewInfo.subresourceRange.baseMipLevel = 0;
	imageViewInfo.subresourceRange.levelCount = 1;
	imageViewInfo.subresourceRange.baseArrayLayer = 0;
	imageViewInfo.subresourceRange.layerCount = 1;
	if (vkCreateImageView(device->device, &imageViewInfo, nullptr, &frame->imageView) != VK_SUCCESS) {
		avAssert(AV_CREATION_ERROR, AV_SUCCESS, "failed to create view in to the swapchain image");
	}
	avAssert(0, 0, "created image view in frame");
}

void frameDestroyResources(RenderDevice device, Window window, Frame frame) {

	vkDestroyImageView(device->device, frame.imageView, nullptr);
	avAssert(0, 0, "destroyed image view in frame");
}


void renderDeviceCreateResources(RenderDevice device, Window window) {
	// swapchain
	SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device->physicalDevice, window);
	VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats, swapChainSupport.formatCount);
	VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes, swapChainSupport.presetnModeCount);
	VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities, window);
	freeSwapchainSupport(swapChainSupport);

	uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
	if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
		imageCount = swapChainSupport.capabilities.maxImageCount;
	}
	uint32 queueFamilyIndices[] = { device->queueFamilyIndices.graphicsFamily, device->queueFamilyIndices.presentFamily };

	VkSwapchainCreateInfoKHR swapchainInfo = { 0 };
	swapchainInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	swapchainInfo.surface = window->surface;
	swapchainInfo.minImageCount = imageCount;
	swapchainInfo.imageFormat = surfaceFormat.format;
	swapchainInfo.imageColorSpace = surfaceFormat.colorSpace;
	swapchainInfo.imageExtent = extent;
	swapchainInfo.imageArrayLayers = 1;
	swapchainInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	if (device->queueFamilyIndices.graphicsFamily != device->queueFamilyIndices.presentFamily) {
		swapchainInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		swapchainInfo.queueFamilyIndexCount = 2;
		swapchainInfo.pQueueFamilyIndices = queueFamilyIndices;
	} else {
		swapchainInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		swapchainInfo.queueFamilyIndexCount = 0;
		swapchainInfo.pQueueFamilyIndices = nullptr;
	}
	swapchainInfo.preTransform = swapChainSupport.capabilities.currentTransform;
	swapchainInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	swapchainInfo.presentMode = presentMode;
	swapchainInfo.clipped = VK_TRUE;
	swapchainInfo.oldSwapchain = VK_NULL_HANDLE;
	VkResult result = vkCreateSwapchainKHR(device->device, &swapchainInfo, nullptr, &window->swapchain);
	if (result != VK_SUCCESS) {
		avAssert(AV_CREATION_ERROR, 0, "creating swapchain");
	}
	avAssert(0, 0, "created swapchain");

	window->frameExtent = extent;
	window->frameFormat = surfaceFormat.format;
	window->frameCount = imageCount;
	window->frames = avAllocate(sizeof(Frame), window->frameCount, "allocating frame data");

	VkCommandPoolCreateInfo poolInfo = { 0 };
	poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	poolInfo.queueFamilyIndex = device->queueFamilyIndices.graphicsFamily;
	if (vkCreateCommandPool(device->device, &poolInfo, nullptr, &window->commandPool) != VK_SUCCESS) {
		avAssert(AV_CREATION_ERROR, 0, "failed to create command pool");
	}
	avAssert(0, 0, "created command pool");

	VkCommandBufferAllocateInfo allocInfo = { 0 };
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = window->commandPool;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandBufferCount = imageCount;
	VkCommandBuffer* commandBuffers = avAllocate(sizeof(VkCommandBuffer), imageCount, "allocating command buffers");
	if (vkAllocateCommandBuffers(device->device, &allocInfo, commandBuffers) != VK_SUCCESS) {
		avAssert(AV_CREATION_ERROR, 0, "failed to allocate commandbuffers");
	}
	avAssert(0, 0, "allocated command commandbuffers");

	vkGetSwapchainImagesKHR(device->device, window->swapchain, &imageCount, nullptr);
	VkImage* swapChainImages = avAllocate(sizeof(VkImage), imageCount, "allocating for swapchain image enumeration");
	vkGetSwapchainImagesKHR(device->device, window->swapchain, &imageCount, swapChainImages);

	FrameCreateInfo frameInfo = { 0 };
	frameInfo.images = swapChainImages;
	frameInfo.cmdBuffers = commandBuffers;

	for (uint i = 0; i < window->frameCount; i++) {
		frameCreateResources(device, window, i, frameInfo, &window->frames[i]);
	}

	avFree(commandBuffers);
	avFree(swapChainImages);


}


void renderDeviceDestroyResources(RenderDevice device, Window window) {

	for (uint i = 0; i < window->frameCount; i++) {
		frameDestroyResources(device, window, window->frames[i]);
	}

	vkDestroyCommandPool(device->device, window->commandPool, nullptr);
	avAssert(0, 0, "destoyed command pool");

	avFree(window->frames);

	vkDestroySwapchainKHR(device->device, window->swapchain, nullptr);
	avAssert(0, 0, "destroyed swapchain");
}

void renderDeviceDestroy(RenderDevice device) {


	vkDestroyDevice(device->device, nullptr);
	avAssert(0, 0, "destroyed render device");

	avFree(device);
}



