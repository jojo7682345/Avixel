#include "../renderer.h"
#include "vulkanShaders.h"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <string.h>
#include <stdio.h>

#undef AV_LOG_CATEGORY
#define AV_LOG_CATEGORY "avixel_renderer"

#define MAX_FRAMES_IN_FLIGHT 2

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

typedef struct Vertex {
	Vec3f pos;
	Vec3f color;
} Vertex;
const VkVertexInputBindingDescription vertexBindingDescription = {
	.inputRate = VK_VERTEX_INPUT_RATE_VERTEX,
	.stride = sizeof(Vertex),
	.binding = 0
};
const VkVertexInputAttributeDescription vertexAttributeDescriptions[] = {
	{
		.binding = 0,
		.format = VK_FORMAT_R32G32B32_SFLOAT,
		.location = 0,
		.offset = offsetof(Vertex, pos)
	},
	{
		.binding = 0,
		.format = VK_FORMAT_R32G32B32_SFLOAT,
		.location = 1,
		.offset = offsetof(Vertex, color)
	},
};
const uint vertexAttributeDescriptionCount = sizeof(vertexAttributeDescriptions) / sizeof(VkVertexInputAttributeDescription);

const Vertex vertices[] = {
	{{.x = -0.5f, .y = -0.5f, .z = 0.0f}, {.r = 1.0f, .g = 0.0f, .b = 0.0f}},
	{{.x =  0.5f, .y = -0.5f, .z = 0.0f}, {.r = 0.0f, .g = 1.0f, .b = 0.0f}},
	{{.x =  0.5f, .y =  0.5f, .z = 0.0f}, {.r = 0.0f, .g = 0.0f, .b = 1.0f}},
	{{.x = -0.5f, .y =  0.5f, .z = 0.0f}, {.r = 1.0f, .g = 1.0f, .b = 1.0f}},
};
const uint16 indices[] = {
	0, 1, 2, 2, 3, 0
};
const uint vertexCount = sizeof(indices) / sizeof(uint16);


typedef struct RenderInstance_T {
	DeviceStatus status;
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

typedef struct Pipeline_T {
	VkPipelineLayout layout;
	VkPipeline pipeline;
}Pipeline_T;

typedef struct DeviceBuffer_T {
	VkBuffer buffer;
	VkDeviceSize size;
	VkDeviceMemory memory;

	RenderDevice device;
} DeviceBuffer_T;

typedef struct RenderResources_T {
	DeviceBuffer_T vertexBuffer;
	DeviceBuffer_T indexBuffer;
} RenderResources_T;

typedef struct RenderDevice_T {
	DeviceStatus status;
	Window window;

	RenderInstance instance;

	VkPhysicalDevice physicalDevice;
	VkDevice device;

	QueueFamilyIndices queueFamilyIndices;
	VkQueue graphicsQueue;
	VkQueue presentQueue;

	VkCommandPool commandPool;

	Pipeline_T renderPipeline;
	Pipeline_T fontPipeline;

	RenderResources_T resources;
} RenderDevice_T;

typedef struct Frame {
	VkFramebuffer framebuffer;
	VkImage image;
	VkImageView imageView;

	VkCommandBuffer* pCommandBuffer;
	VkCommandPool* commandPool;

	VkSemaphore imageAvailable;
	VkSemaphore renderFinished;
	VkFence inFlight;

	VkFormat* format;
	VkExtent2D* extent;
}Frame;

typedef struct Window_T {
	DeviceStatus status;
	AvInstance instance;

	DisplaySurface displaySurface;
	GLFWwindow* window;

	VkSurfaceKHR surface;

	VkSwapchainKHR swapchain;

	VkExtent2D frameExtent;
	VkFormat frameFormat;
	VkColorSpaceKHR frameColorspace;
	VkPresentModeKHR framePresentMode;
	VkSurfaceTransformFlagBitsKHR frameTransform;

	uint frameCount;
	Frame* frames;
	uint frameIndex;
	uint nextFrameIndex;

	VkRenderPass renderPass;

	VkCommandBuffer* commandBuffers;

	void (*onWindowResize)(AvWindow window, uint width, uint height);
	void (*onWindowDisconnect)(AvWindow window);
} Window_T;

typedef struct DisplaySurface_T {
	DeviceStatus status;
	uint width;
	uint height;
	DisplayType type;
} DisplaySurface_T;





void checkCreation_(VkResult result, const char* msg, AV_LOCATION_ARGS, AV_CATEGORY_ARGS) {
	if (result != VK_SUCCESS) {
		avAssert_(AV_CREATION_ERROR, AV_SUCCESS, line, file, func, category, msg);
	}
}
#define checkCreation(result, msg) checkCreation_(result,msg, AV_LOCATION_PARAMS, AV_LOG_CATEGORY)

RendererType getRendererType() {
	return RENDERER_TYPE_VULKAN;
}

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
	avLog(AV_DEBUG_INFO, monitorSize);

	avLog(AV_DEBUG_CREATE, "initialized display surface");

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

	avLog(AV_DEBUG_DESTROY, "deinitialized display surface");
}

const char** displaySurfaceEnumerateExtensions(AvInstance instance, uint* count) {
	return glfwGetRequiredInstanceExtensions(count);
}

void onWindowResize(GLFWwindow* glfwWindow, int width, int height) {
	Window window = (Window)glfwGetWindowUserPointer(glfwWindow);

	window->status |= DEVICE_STATUS_RESIZED;

	if (width == 0 || height == 0) {
		window->status |= DEVICE_STATUS_INOPERABLE;
		avLog(AV_WINDOW_SIZE, "window minimized");
	} else {
		avLog(AV_WINDOW_SIZE, "window no longer minimized");
		window->status &= ~DEVICE_STATUS_INOPERABLE;
	}

	if (window->onWindowResize) {
		window->onWindowResize(nullptr, width, height);
	}
}

void onWindowCloseRequest(GLFWwindow* glfwWindow) {
	Window window = (Window)glfwGetWindowUserPointer(glfwWindow);

	window->status |= DEVICE_STATUS_SHUTDOWN_REQUESTED;

	if (window->onWindowDisconnect) {
		window->onWindowDisconnect(nullptr);
	}

}

AvResult displaySurfaceCreateWindow(AvInstance instance, WindowCreateInfo windowCreateInfo, WindowProperties* windowProperties, Window* window) {

	if (windowCreateInfo.properties.size.width > instance->displaySurface->width || !windowCreateInfo.properties.size.width) {
		windowCreateInfo.properties.size.width = instance->displaySurface->width;
	}
	if (windowCreateInfo.properties.size.height > instance->displaySurface->height || !windowCreateInfo.properties.size.height) {
		windowCreateInfo.properties.size.height = instance->displaySurface->height;
	}

	*window = avAllocate(sizeof(Window_T), 1, "allocating window handle");
	(*window)->instance = instance;
	(*window)->displaySurface = instance->displaySurface;


	GLFWmonitor* fullscreen = windowCreateInfo.properties.fullSurface ? glfwGetPrimaryMonitor() : NULL;

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, windowCreateInfo.properties.resizable ? GLFW_TRUE : GLFW_FALSE);
	glfwWindowHint(GLFW_DECORATED, windowCreateInfo.properties.decorated ? GLFW_TRUE : GLFW_FALSE);
	(*window)->window = glfwCreateWindow(windowCreateInfo.properties.size.width, windowCreateInfo.properties.size.height, windowCreateInfo.properties.title, fullscreen, NULL);
	if ((*window)->window == NULL) {
		avAssert(AV_CREATION_ERROR, 0, "Failed to create the window");
	}
	avLog(AV_DEBUG_CREATE, "created window");

	glfwSetWindowUserPointer((*window)->window, *window);

	if (windowCreateInfo.properties.size.x != -1 && windowCreateInfo.properties.size.y != -1) {
		uint x = windowCreateInfo.properties.size.x;
		uint y = windowCreateInfo.properties.size.y;

		if (x == -2) {
			x = instance->displaySurface->width / 2 - windowCreateInfo.properties.size.width / 2;
		}
		if (y == -2) {
			y = instance->displaySurface->height / 2 - windowCreateInfo.properties.size.height / 2;
		}

		glfwSetWindowPos((*window)->window, x, y);
	} else if ((windowCreateInfo.properties.size.x == -1 && windowCreateInfo.properties.size.y != -1) ||
		(windowCreateInfo.properties.size.x != -1 && windowCreateInfo.properties.size.y == -1)) {
		avAssert(AV_UNUSUAL_ARGUMENTS, AV_SUCCESS, "single axis specified, should be both or none");
	}

	glfwShowWindow((*window)->window);

	glfwSetWindowCloseCallback((*window)->window, onWindowCloseRequest);
	glfwSetFramebufferSizeCallback((*window)->window, onWindowResize);

	if (windowCreateInfo.properties.resizable && !windowCreateInfo.onWindowResize) {
		avAssert(AV_UNSPECIFIED_CALLBACK, AV_SUCCESS, "window is resizable, but no resize callback is specified");
	}
	(*window)->onWindowResize = windowCreateInfo.onWindowResize;
	(*window)->onWindowDisconnect = windowCreateInfo.onWindowDisconnect;

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

	avLog(AV_DEBUG_CREATE, "created window surface");
	return AV_SUCCESS;
}

void windowUpdateEvents(Window window) {
	glfwPollEvents();
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

	AvValidationLevel level = { 0 };
	if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT) {
		level = AV_VALIDATION_LEVEL_VERBOSE;
	}
	if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT) {
		level = AV_VALIDATION_LEVEL_INFO;
	}
	if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
		level = AV_VALIDATION_LEVEL_WARNINGS_AND_ERRORS;
	}
	if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) {
		level = AV_VALIDATION_LEVEL_ERRORS;
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
	avLog(AV_DEBUG_CREATE, "created the vulkan instance");
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
		avLog(AV_DEBUG_CREATE, "created the validation logger");
	}
}

void renderInstanceDestroy(AvInstance instance) {

	if (instance->window) {
		displaySurfaceDestroyWindow(instance, instance->window);
		avLog(AV_DEBUG_DESTROY, "destroyed primary window");
	}

	if (instance->renderInstance->debugMessenger) {
		destroyDebugUtilsMessengerEXT(instance->renderInstance->instance, *(instance->renderInstance->debugMessenger), NULL);
		avFree(instance->renderInstance->debugMessenger);
		avLog(AV_DEBUG_DESTROY, "destroyed validation logger");
	}

	vkDestroyInstance(instance->renderInstance->instance, NULL);
	avLog(AV_DEBUG_DESTROY, "destroyed vulkan instance");

	avFree(instance->renderInstance);
}

DeviceStatus renderInstanceGetStatus(RenderInstance instance) {
	return instance->status;
}

DeviceStatus renderDeviceGetStatus(RenderDevice device) {
	return device->status;
}

DeviceStatus displaySurfaceGetStatus(DisplaySurface instance) {
	return instance->status;
}

DeviceStatus windowGetStatus(Window window) {
	return window->status;
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

void renderDeviceCreate(AvInstance instance, RenderDeviceCreateInfo createInfo, RenderDevice* pDevice) {
	Window window = createInfo.window;

	*pDevice = avAllocate(sizeof(RenderDevice_T), 1, "allocating render instance");
	(*pDevice)->instance = instance->renderInstance;
	(*pDevice)->window = window;

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
		avLog(AV_DEBUG_INFO, msg);

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
		avLog(AV_DEBUG_INFO, msg);
	}
	avLog(AV_DEBUG_SUCCESS, "found physical device");

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

	VkDeviceCreateInfo deviceCreateInfo = { 0 };
	deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	deviceCreateInfo.pQueueCreateInfos = queueCreateInfos;
	deviceCreateInfo.queueCreateInfoCount = queueCount;
	deviceCreateInfo.pEnabledFeatures = &deviceFeatures;

	deviceCreateInfo.enabledExtensionCount = deviceExtensionCount;
	deviceCreateInfo.ppEnabledExtensionNames = deviceExtensions;

	if (instance->renderInstance->debugMessenger) {
		deviceCreateInfo.enabledLayerCount = validationLayerCount;
		deviceCreateInfo.ppEnabledLayerNames = validationLayers;
	} else {
		deviceCreateInfo.enabledLayerCount = 0;
	}
	VkResult result;
	result = vkCreateDevice((*pDevice)->physicalDevice, &deviceCreateInfo, nullptr, &(*pDevice)->device);
	if (result != VK_SUCCESS) {
		avAssert(AV_CREATION_ERROR, AV_SUCCESS, "creating the vulkan logical device");
	}
	avLog(AV_DEBUG_CREATE, "created render device");

	vkGetDeviceQueue((*pDevice)->device, indices.graphicsFamily, 0, &(*pDevice)->graphicsQueue);
	vkGetDeviceQueue((*pDevice)->device, indices.presentFamily, 0, &(*pDevice)->presentQueue);

}

void frameCreateSwapchainResources(RenderDevice device, uint frameIndex, VkImage* images, Frame* frame) {
	frame->image = images[frameIndex];

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
	avLog(AV_DEBUG_CREATE, "created image view in frame");

	VkFramebufferCreateInfo framebufferInfo = { 0 };
	framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	framebufferInfo.renderPass = device->window->renderPass;
	framebufferInfo.attachmentCount = 1;
	framebufferInfo.pAttachments = &frame->imageView;
	framebufferInfo.width = frame->extent->width;
	framebufferInfo.height = frame->extent->height;
	framebufferInfo.layers = 1;

	checkCreation(
		vkCreateFramebuffer(device->device, &framebufferInfo, nullptr, &frame->framebuffer),
		"creating framebuffer"
	);
	avLog(AV_DEBUG_CREATE, "created framebuffer");
}

void frameDestroySwapchainResources(RenderDevice device, Frame frame) {
	vkDestroyImageView(device->device, frame.imageView, nullptr);
	avLog(AV_DEBUG_DESTROY, "destroyed image view in frame");

	vkDestroyFramebuffer(device->device, frame.framebuffer, nullptr);
	avLog(AV_DEBUG_DESTROY, "destroyed framebuffer");
}

void createSwapchain(RenderDevice device) {

	uint imageCount = device->window->frameCount;

	uint32 queueFamilyIndices[] = { device->queueFamilyIndices.graphicsFamily, device->queueFamilyIndices.presentFamily };

	VkSwapchainCreateInfoKHR swapchainInfo = { 0 };
	swapchainInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	swapchainInfo.surface = device->window->surface;
	swapchainInfo.minImageCount = imageCount;
	swapchainInfo.imageFormat = device->window->frameFormat;
	swapchainInfo.imageColorSpace = device->window->frameColorspace;
	swapchainInfo.imageExtent = device->window->frameExtent;
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
	swapchainInfo.preTransform = device->window->frameTransform;
	swapchainInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	swapchainInfo.presentMode = device->window->framePresentMode;
	swapchainInfo.clipped = VK_TRUE;
	swapchainInfo.oldSwapchain = VK_NULL_HANDLE;
	VkResult result = vkCreateSwapchainKHR(device->device, &swapchainInfo, nullptr, &device->window->swapchain);
	if (result != VK_SUCCESS) {
		avAssert(AV_CREATION_ERROR, 0, "creating swapchain");
	}
	avLog(AV_DEBUG_CREATE, "created swapchain");

	vkGetSwapchainImagesKHR(device->device, device->window->swapchain, &imageCount, nullptr);
	VkImage* swapChainImages = avAllocate(sizeof(VkImage), imageCount, "allocating for swapchain image enumeration");
	vkGetSwapchainImagesKHR(device->device, device->window->swapchain, &imageCount, swapChainImages);

	for (uint i = 0; i < device->window->frameCount; i++) {
		frameCreateSwapchainResources(device, i, swapChainImages, &device->window->frames[i]);
	}

	avFree(swapChainImages);


}

void cleanupSwapChain(RenderDevice device) {
	for (uint i = 0; i < device->window->frameCount; i++) {
		frameDestroySwapchainResources(device, device->window->frames[i]);
	}

	vkDestroySwapchainKHR(device->device, device->window->swapchain, nullptr);
}

void recreateSwapchain(RenderDevice device) {
	avLog(AV_SWAPCHAIN_RECREATION, "recreating swapchain");

	vkDeviceWaitIdle(device->device);
	cleanupSwapChain(device);
	createSwapchain(device);

}

void frameCreateResources(RenderDevice device, Window window, uint frameIndex, Frame* frame) {

	frame->extent = &window->frameExtent;
	frame->format = &window->frameFormat;
	frame->commandPool = &device->commandPool;
	frame->pCommandBuffer = &window->commandBuffers[frameIndex];

	VkSemaphoreCreateInfo semaphoreCreateInfo = { 0 };
	semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	semaphoreCreateInfo.flags = 0;
	semaphoreCreateInfo.pNext = nullptr;

	checkCreation(
		vkCreateSemaphore(device->device, &semaphoreCreateInfo, nullptr, &frame->imageAvailable),
		"creating image available semaphore"
	);
	avLog(AV_DEBUG_CREATE, "created image available semaphore");

	checkCreation(
		vkCreateSemaphore(device->device, &semaphoreCreateInfo, nullptr, &frame->renderFinished),
		"creating render finished semaphore"
	);
	avLog(AV_DEBUG_CREATE, "created render finished semaphore");

	VkFenceCreateInfo fenceCreateInfo = { 0 };
	fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
	fenceCreateInfo.pNext = nullptr;

	checkCreation(
		vkCreateFence(device->device, &fenceCreateInfo, nullptr, &frame->inFlight),
		"creating in flight fence"
	);
	avLog(AV_DEBUG_CREATE, "created in flight fence");
}

void frameDestroyResources(RenderDevice device, Window window, Frame frame) {

	vkDestroySemaphore(device->device, frame.imageAvailable, nullptr);
	avLog(AV_DEBUG_DESTROY, "destroyed image available semaphore");

	vkDestroySemaphore(device->device, frame.renderFinished, nullptr);
	avLog(AV_DEBUG_DESTROY, "destroyed render finished semaphore");

	vkDestroyFence(device->device, frame.inFlight, nullptr);
	avLog(AV_DEBUG_DESTROY, "destroyed in flight fence");

}

uint32_t findMemoryType(RenderDevice device, uint32_t typeFilter, VkMemoryPropertyFlags properties) {
	VkPhysicalDeviceMemoryProperties memProperties;
	vkGetPhysicalDeviceMemoryProperties(device->physicalDevice, &memProperties);
	for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
		if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
			return i;
		}
	}
	avAssert(AV_MEMORY_ERROR, AV_SUCCESS, "failed to find suitable memory type");
	return -1;
}

void createDeviceBuffer(RenderDevice device, uint size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, DeviceBuffer_T* buffer) {
	buffer->device = device;
	buffer->size = size;

	VkBufferCreateInfo bufferInfo = { 0 };
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.size = size;
	bufferInfo.usage = usage;
	bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	checkCreation(
		vkCreateBuffer(device->device, &bufferInfo, nullptr, &buffer->buffer),
		"creating buffer"
	);

	VkMemoryRequirements memRequirements;
	vkGetBufferMemoryRequirements(device->device, buffer->buffer, &memRequirements);

	VkMemoryAllocateInfo allocInfo = { 0 };
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = findMemoryType(device, memRequirements.memoryTypeBits, properties);

	checkCreation(
		vkAllocateMemory(device->device, &allocInfo, nullptr, &buffer->memory),
		"allocating buffer memory"
	);

	vkBindBufferMemory(device->device, buffer->buffer, buffer->memory, 0);
}

void deviceBufferPush(DeviceBuffer_T buffer, const void* const data) {
	void* bufferPtr;
	vkMapMemory(buffer.device->device, buffer.memory, 0, buffer.size, 0, &bufferPtr);
	memcpy(bufferPtr, data, buffer.size);
	vkUnmapMemory(buffer.device->device, buffer.memory);
}

void deviceBufferPull(DeviceBuffer_T buffer, void* data) {
	void* bufferPtr;
	vkMapMemory(buffer.device->device, buffer.memory, 0, buffer.size, 0, &bufferPtr);
	memcpy(data, bufferPtr, buffer.size);
	vkUnmapMemory(buffer.device->device, buffer.memory);
}

void destroyDeviceBuffer(DeviceBuffer_T buffer) {
	vkDestroyBuffer(buffer.device->device, buffer.buffer, nullptr);
	vkFreeMemory(buffer.device->device, buffer.memory, nullptr);
}

void copyDeviceBuffer(DeviceBuffer_T src, DeviceBuffer_T dst) {
	if (src.device != dst.device) {
		avAssert(AV_DEVICE_MISMATCH, 0, "buffers created on different devices");
		return;
	}
	if (src.size != dst.size) {
		avAssert(AV_INVALID_SIZE, 0, "buffers differ in size");
		return;
	}
	VkCommandBufferAllocateInfo allocInfo = { 0 };
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandPool = src.device->commandPool;
	allocInfo.commandBufferCount = 1;

	VkCommandBuffer commandBuffer;
	vkAllocateCommandBuffers(src.device->device, &allocInfo, &commandBuffer);

	VkCommandBufferBeginInfo beginInfo = { 0 };
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	vkBeginCommandBuffer(commandBuffer, &beginInfo);

	VkBufferCopy copyRegion = { 0 };
	copyRegion.srcOffset = 0; // Optional
	copyRegion.dstOffset = 0; // Optional
	copyRegion.size = src.size;
	vkCmdCopyBuffer(commandBuffer, src.buffer, dst.buffer, 1, &copyRegion);

	vkEndCommandBuffer(commandBuffer);

	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;

	vkQueueSubmit(src.device->graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
	vkQueueWaitIdle(src.device->graphicsQueue);
	vkFreeCommandBuffers(src.device->device, src.device->commandPool, 1, &commandBuffer);

}

void renderDeviceCreateRenderResources(RenderDevice device) {
	Window window = device->window;

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

	window->frameExtent = extent;
	window->frameFormat = surfaceFormat.format;
	window->frameColorspace = surfaceFormat.colorSpace;
	window->frameTransform = swapChainSupport.capabilities.currentTransform;
	window->frameCount = imageCount;
	window->frames = avAllocate(sizeof(Frame), window->frameCount, "allocating frame data");

	VkCommandPoolCreateInfo poolInfo = { 0 };
	poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	poolInfo.queueFamilyIndex = device->queueFamilyIndices.graphicsFamily;
	if (vkCreateCommandPool(device->device, &poolInfo, nullptr, &device->commandPool) != VK_SUCCESS) {
		avAssert(AV_CREATION_ERROR, 0, "failed to create command pool");
	}
	avLog(AV_DEBUG_CREATE, "created command pool");

	VkCommandBufferAllocateInfo allocInfo = { 0 };
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = device->commandPool;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandBufferCount = imageCount;
	window->commandBuffers = avAllocate(sizeof(VkCommandBuffer), imageCount, "allocating command buffers");
	if (vkAllocateCommandBuffers(device->device, &allocInfo, window->commandBuffers) != VK_SUCCESS) {
		avAssert(AV_CREATION_ERROR, 0, "failed to allocate commandbuffers");
	}
	avLog(AV_DEBUG_CREATE, "allocated command commandbuffers");

	// renderpass 
	VkAttachmentDescription colorAttachmentDescription = { 0 };
	colorAttachmentDescription.format = window->frameFormat;
	colorAttachmentDescription.samples = VK_SAMPLE_COUNT_1_BIT;
	colorAttachmentDescription.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	colorAttachmentDescription.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachmentDescription.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachmentDescription.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorAttachmentDescription.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colorAttachmentDescription.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	VkAttachmentReference colorAttachmentReference = { 0 };
	colorAttachmentReference.attachment = 0;
	colorAttachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass = { 0 };
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorAttachmentReference;

	VkSubpassDependency dependency = { 0 };
	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	dependency.dstSubpass = 0;
	dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.srcAccessMask = 0;
	dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

	VkRenderPassCreateInfo renderPassInfo = { 0 };
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.pNext = nullptr;
	renderPassInfo.flags = 0;
	renderPassInfo.attachmentCount = 1;
	renderPassInfo.pAttachments = &colorAttachmentDescription;
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;
	renderPassInfo.dependencyCount = 1;
	renderPassInfo.pDependencies = &dependency;

	if (vkCreateRenderPass(device->device, &renderPassInfo, nullptr, &(window->renderPass)) != VK_SUCCESS) {
		avAssert(AV_CREATION_ERROR, AV_SUCCESS, "creating renderpass");
	}
	avLog(AV_DEBUG_CREATE, "created renderpass");

	for (uint i = 0; i < device->window->frameCount; i++) {
		frameCreateResources(device, device->window, i, &device->window->frames[i]);
	}

	createSwapchain(device);


	uint vertexBufferSize = sizeof(vertices);
	DeviceBuffer_T stagingBuffer;
	createDeviceBuffer(
		device,
		vertexBufferSize,
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		&stagingBuffer
	);
	deviceBufferPush(stagingBuffer, vertices);
	createDeviceBuffer(
		device,
		vertexBufferSize,
		VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		&device->resources.vertexBuffer
	);
	copyDeviceBuffer(stagingBuffer, device->resources.vertexBuffer);
	destroyDeviceBuffer(stagingBuffer);

	uint indexBufferSize = sizeof(indices);
	createDeviceBuffer(
		device,
		indexBufferSize,
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		&stagingBuffer
	);
	deviceBufferPush(stagingBuffer, indices);
	createDeviceBuffer(
		device,
		indexBufferSize,
		VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		&device->resources.indexBuffer
	);
	copyDeviceBuffer(stagingBuffer, device->resources.indexBuffer);
	destroyDeviceBuffer(stagingBuffer);

}

VkShaderModule createShaderModule(const size_t codeSize, const char* codeText, RenderDevice device, const char* msg) {
	VkShaderModuleCreateInfo shaderModuleCreateInfo = { 0 };
	shaderModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	shaderModuleCreateInfo.codeSize = codeSize;
	shaderModuleCreateInfo.pCode = (uint*)codeText;
	shaderModuleCreateInfo.flags = 0;
	shaderModuleCreateInfo.pNext = nullptr;

	VkShaderModule shaderModule;

	if (vkCreateShaderModule(device->device, &shaderModuleCreateInfo, nullptr, &shaderModule) != VK_SUCCESS) {
		avAssert(AV_CREATION_ERROR, AV_SUCCESS, msg);
	}
	avLog(AV_DEBUG_CREATE, "created shader module");

	return shaderModule;
}


void renderDeviceCreatePipelines(RenderDevice device, uint createInfoCount, PipelineCreateInfo* createInfos) {

	Window window = device->window;


	VkPipelineLayoutCreateInfo renderPipelineLayoutCreateInfo = { 0 };
	renderPipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	renderPipelineLayoutCreateInfo.setLayoutCount = 0; // Optional
	renderPipelineLayoutCreateInfo.pSetLayouts = nullptr; // Optional
	renderPipelineLayoutCreateInfo.pushConstantRangeCount = 0; // Optional
	renderPipelineLayoutCreateInfo.pPushConstantRanges = nullptr; // Optional

	VkPipelineLayoutCreateInfo fontPipelineLayoutCreateInfo = { 0 };
	fontPipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	fontPipelineLayoutCreateInfo.setLayoutCount = 0; // Optional
	fontPipelineLayoutCreateInfo.pSetLayouts = nullptr; // Optional
	fontPipelineLayoutCreateInfo.pushConstantRangeCount = 0; // Optional
	fontPipelineLayoutCreateInfo.pPushConstantRanges = nullptr; // Optional

	checkCreation(
		vkCreatePipelineLayout(device->device, &renderPipelineLayoutCreateInfo, nullptr, &device->renderPipeline.layout),
		"creating render pipeline layout"
	);
	avLog(AV_DEBUG_CREATE, "created render pipeline layout");

	checkCreation(
		vkCreatePipelineLayout(device->device, &fontPipelineLayoutCreateInfo, nullptr, &device->fontPipeline.layout),
		"creating font pipeline layout"
	);
	avLog(AV_DEBUG_CREATE, "created font pipeline layout");

	VkPipelineVertexInputStateCreateInfo vertexInputInfo = { 0 };
	vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputInfo.vertexBindingDescriptionCount = 1;
	vertexInputInfo.pVertexBindingDescriptions = &vertexBindingDescription;
	vertexInputInfo.vertexAttributeDescriptionCount = vertexAttributeDescriptionCount;
	vertexInputInfo.pVertexAttributeDescriptions = vertexAttributeDescriptions;

	VkPipelineInputAssemblyStateCreateInfo inputAssembly = { 0 };
	inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	inputAssembly.primitiveRestartEnable = VK_FALSE;

	VkViewport viewport = { 0 };
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = (float)window->frameExtent.width;
	viewport.height = (float)window->frameExtent.height;

	VkRect2D scissor = { 0 };
	scissor.offset.x = 0;
	scissor.offset.y = 0;
	scissor.extent = window->frameExtent;

	VkPipelineViewportStateCreateInfo viewportState = { 0 };
	viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportState.viewportCount = 1;
	viewportState.pViewports = &viewport;
	viewportState.scissorCount = 1;
	viewportState.pScissors = &scissor;

	VkPipelineRasterizationStateCreateInfo rasterizer = { 0 };
	rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizer.depthClampEnable = VK_FALSE;
	rasterizer.rasterizerDiscardEnable = VK_FALSE;
	rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
	rasterizer.lineWidth = 1.0f;
	rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
	rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
	rasterizer.depthBiasEnable = VK_FALSE;
	rasterizer.depthBiasConstantFactor = 0.0f; // Optional
	rasterizer.depthBiasClamp = 0.0f; // Optional
	rasterizer.depthBiasSlopeFactor = 0.0f; // Optional

	VkPipelineMultisampleStateCreateInfo multisampling = { 0 };
	multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampling.sampleShadingEnable = VK_FALSE;
	multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	multisampling.minSampleShading = 1.0f; // Optional
	multisampling.pSampleMask = nullptr; // Optional
	multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
	multisampling.alphaToOneEnable = VK_FALSE; // Optional

	VkPipelineColorBlendAttachmentState colorBlendAttachment = { 0 };
	colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	colorBlendAttachment.blendEnable = VK_TRUE;
	colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
	colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
	colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
	colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
	colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
	colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

	VkPipelineColorBlendStateCreateInfo colorBlending = { 0 };
	colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlending.logicOpEnable = VK_FALSE;
	colorBlending.logicOp = VK_LOGIC_OP_COPY; // Optional
	colorBlending.attachmentCount = 1;
	colorBlending.pAttachments = &colorBlendAttachment;
	colorBlending.blendConstants[0] = 0.0f; // Optional
	colorBlending.blendConstants[1] = 0.0f; // Optional
	colorBlending.blendConstants[2] = 0.0f; // Optional
	colorBlending.blendConstants[3] = 0.0f; // Optional

	VkDynamicState dynamicStates[] = {
			VK_DYNAMIC_STATE_VIEWPORT,
			VK_DYNAMIC_STATE_SCISSOR
	};
	VkPipelineDynamicStateCreateInfo dynamicState = { 0 };
	dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamicState.dynamicStateCount = sizeof(dynamicStates) / sizeof(VkDynamicState);
	dynamicState.pDynamicStates = dynamicStates;


	// creating shader modules
	VkShaderModule fontShaderVertModule = createShaderModule(font_shader_vert_size, font_shader_vert_data, device, "creating font vertex shader");
	VkShaderModule fontShaderFragModule = createShaderModule(font_shader_frag_size, font_shader_frag_data, device, "creating font fragment shader");
	VkShaderModule basicShaderVertModule = createShaderModule(basic_shader_vert_size, basic_shader_vert_data, device, "creating basic vertex shader");
	VkShaderModule basicShaderFragModule = createShaderModule(basic_shader_frag_size, basic_shader_frag_data, device, "creating basic fragment shader");

	VkPipelineShaderStageCreateInfo renderShaderVertInfo = { 0 };
	renderShaderVertInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	renderShaderVertInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
	renderShaderVertInfo.module = basicShaderVertModule;
	renderShaderVertInfo.pName = "main";

	VkPipelineShaderStageCreateInfo renderShaderFragInfo = { 0 };
	renderShaderFragInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	renderShaderFragInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	renderShaderFragInfo.module = basicShaderFragModule;
	renderShaderFragInfo.pName = "main";

	VkPipelineShaderStageCreateInfo fontShaderVertInfo = { 0 };
	fontShaderVertInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	fontShaderVertInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
	fontShaderVertInfo.module = fontShaderVertModule;
	fontShaderVertInfo.pName = "main";

	VkPipelineShaderStageCreateInfo fontShaderFragInfo = { 0 };
	fontShaderFragInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	fontShaderFragInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	fontShaderFragInfo.module = fontShaderFragModule;
	fontShaderFragInfo.pName = "main";

	VkPipelineShaderStageCreateInfo renderShaderStages[] = { renderShaderVertInfo, renderShaderFragInfo };
	VkPipelineShaderStageCreateInfo fontShaderStages[] = { fontShaderVertInfo, fontShaderFragInfo };

	// creating the pipeline
	VkGraphicsPipelineCreateInfo renderPipelineInfo = { 0 };
	renderPipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	renderPipelineInfo.stageCount = sizeof(renderShaderStages) / sizeof(VkPipelineShaderStageCreateInfo);
	renderPipelineInfo.pStages = renderShaderStages;

	renderPipelineInfo.pVertexInputState = &vertexInputInfo;
	renderPipelineInfo.pInputAssemblyState = &inputAssembly;
	renderPipelineInfo.pViewportState = &viewportState;
	renderPipelineInfo.pRasterizationState = &rasterizer;
	renderPipelineInfo.pMultisampleState = &multisampling;
	renderPipelineInfo.pDepthStencilState = nullptr;
	renderPipelineInfo.pColorBlendState = &colorBlending;
	renderPipelineInfo.pDynamicState = &dynamicState;

	renderPipelineInfo.layout = device->renderPipeline.layout;
	renderPipelineInfo.renderPass = window->renderPass;
	renderPipelineInfo.subpass = 0;

	renderPipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
	renderPipelineInfo.basePipelineIndex = -1;
	renderPipelineInfo.flags = VK_PIPELINE_CREATE_ALLOW_DERIVATIVES_BIT;

	VkGraphicsPipelineCreateInfo fontPipelineInfo = { 0 };
	fontPipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	fontPipelineInfo.stageCount = sizeof(fontShaderStages) / sizeof(VkPipelineShaderStageCreateInfo);
	fontPipelineInfo.pStages = fontShaderStages;

	fontPipelineInfo.pVertexInputState = &vertexInputInfo;
	fontPipelineInfo.pInputAssemblyState = &inputAssembly;
	fontPipelineInfo.pViewportState = &viewportState;
	fontPipelineInfo.pRasterizationState = &rasterizer;
	fontPipelineInfo.pMultisampleState = &multisampling;
	fontPipelineInfo.pDepthStencilState = nullptr;
	fontPipelineInfo.pColorBlendState = &colorBlending;
	fontPipelineInfo.pDynamicState = &dynamicState;

	fontPipelineInfo.layout = device->fontPipeline.layout;
	fontPipelineInfo.renderPass = window->renderPass;
	fontPipelineInfo.subpass = 0;

	fontPipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
	fontPipelineInfo.basePipelineIndex = 0;
	fontPipelineInfo.flags = VK_PIPELINE_CREATE_DERIVATIVE_BIT;

	VkPipeline pipelines[] = { VK_NULL_HANDLE, VK_NULL_HANDLE };
	VkGraphicsPipelineCreateInfo pipelineCreateInfos[] = { renderPipelineInfo, fontPipelineInfo };
	uint pipelineCreateInfoCount = sizeof(pipelineCreateInfos) / sizeof(VkGraphicsPipelineCreateInfo);


	checkCreation(
		vkCreateGraphicsPipelines(device->device, VK_NULL_HANDLE, pipelineCreateInfoCount, pipelineCreateInfos, nullptr, pipelines),
		"creating render pipelines"
	);
	avLog(AV_DEBUG_CREATE, "created pipelines");

	device->renderPipeline.pipeline = pipelines[0];
	device->fontPipeline.pipeline = pipelines[1];


	vkDestroyShaderModule(device->device, basicShaderVertModule, nullptr);
	vkDestroyShaderModule(device->device, basicShaderFragModule, nullptr);
	vkDestroyShaderModule(device->device, fontShaderVertModule, nullptr);
	vkDestroyShaderModule(device->device, fontShaderFragModule, nullptr);
}

void renderDeviceWaitIdle(RenderDevice device) {
	vkDeviceWaitIdle(device->device);
}

AvResult renderDeviceAquireNextFrame(RenderDevice device) {
	uint frameIndex = device->window->frameIndex;
	vkWaitForFences(device->device, 1, &device->window->frames[frameIndex].inFlight, VK_TRUE, UINT64_MAX);

	VkResult result = vkAcquireNextImageKHR(
		device->device,
		device->window->swapchain,
		UINT64_MAX,
		device->window->frames[frameIndex].imageAvailable,
		VK_NULL_HANDLE,
		&device->window->nextFrameIndex
	);

	if (result == VK_ERROR_OUT_OF_DATE_KHR) {
		recreateSwapchain(device);
		return AV_SUCCESS;
	} else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
		avAssert(AV_SWAPCHAIN_ERROR, AV_SUCCESS, "failed to aquire swapchain image");
	}

	vkResetFences(device->device, 1, &device->window->frames[frameIndex].inFlight);

	return AV_SUCCESS;
}

AvResult renderDeviceRecordRenderCommands(RenderDevice device, RenderCommandsInfo commands) {
	uint imageIndex = device->window->frameIndex;
	VkCommandBuffer commandBuffer = *(device->window->frames[imageIndex].pCommandBuffer);

	vkResetCommandBuffer(commandBuffer, 0);

	VkCommandBufferBeginInfo beginInfo = { 0 };
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = 0; // Optional
	beginInfo.pInheritanceInfo = nullptr; // Optional

	if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
		avAssert(AV_RENDER_COMMAND_ERROR, AV_SUCCESS, "command recording begin failed");
	}

	VkRenderPassBeginInfo renderPassInfo = { 0 };
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassInfo.renderPass = device->window->renderPass;
	renderPassInfo.framebuffer = device->window->frames[device->window->nextFrameIndex].framebuffer;
	renderPassInfo.renderArea.offset.x = 0;
	renderPassInfo.renderArea.offset.y = 0;
	renderPassInfo.renderArea.extent = device->window->frameExtent;

	VkClearValue clearColor = { {{0.0f, 0.0f, 0.0f, 1.0f}} };
	renderPassInfo.clearValueCount = 1;
	renderPassInfo.pClearValues = &clearColor;

	vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, device->renderPipeline.pipeline);

	VkViewport viewport = { 0 };
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = device->window->frameExtent.width;
	viewport.height = device->window->frameExtent.height;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;
	vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

	VkRect2D scissor = { 0 };
	scissor.offset.x = 0;
	scissor.offset.y = 0;
	scissor.extent = device->window->frameExtent;
	vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

	VkBuffer vertexBuffers[] = { device->resources.vertexBuffer.buffer };
	VkDeviceSize offsets[] = { 0 };
	vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
	vkCmdBindIndexBuffer(commandBuffer, device->resources.indexBuffer.buffer, 0, VK_INDEX_TYPE_UINT16);
	vkCmdDrawIndexed(commandBuffer, vertexCount, 1, 0, 0, 0);

	vkCmdEndRenderPass(commandBuffer);

	if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
		avAssert(AV_RENDER_COMMAND_ERROR, AV_SUCCESS, "failed command buffer recording");
	}
	return AV_SUCCESS;
}

AvResult renderDeviceRenderFrame(RenderDevice device) {

	uint imageIndex = device->window->frameIndex;

	VkSubmitInfo submitInfo = { 0 };
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

	VkSemaphore waitSemaphores[] = { device->window->frames[imageIndex].imageAvailable };
	VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = waitSemaphores;
	submitInfo.pWaitDstStageMask = waitStages;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = device->window->frames[imageIndex].pCommandBuffer;

	VkSemaphore signalSemaphores[] = { device->window->frames[imageIndex].renderFinished };
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = signalSemaphores;

	if (vkQueueSubmit(device->graphicsQueue, 1, &submitInfo, device->window->frames[imageIndex].inFlight) != VK_SUCCESS) {
		avAssert(AV_RENDER_ERROR, AV_SUCCESS, "failed render submission");
	}
	return AV_SUCCESS;
}

AvResult renderDevicePresent(RenderDevice device) {

	uint imageIndex = device->window->frameIndex;

	VkPresentInfoKHR presentInfo = { 0 };
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

	presentInfo.waitSemaphoreCount = 1;
	VkSemaphore signalSemaphores[] = { device->window->frames[imageIndex].renderFinished };
	presentInfo.pWaitSemaphores = signalSemaphores;

	VkSwapchainKHR swapChains[] = { device->window->swapchain };
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = swapChains;

	presentInfo.pImageIndices = &device->window->nextFrameIndex;

	VkResult result = vkQueuePresentKHR(device->presentQueue, &presentInfo);
	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || device->window->status & (DEVICE_STATUS_RESIZED | DEVICE_STATUS_INOPERABLE)) {
		recreateSwapchain(device);
		device->window->status &= ~DEVICE_STATUS_RESIZED;
		device->window->status &= ~DEVICE_STATUS_INOPERABLE;
	} else if (result != VK_SUCCESS) {
		avAssert(AV_PRESENT_ERROR, AV_SUCCESS, "failed to present");
	}

	device->window->frameIndex = device->window->nextFrameIndex;

	return AV_SUCCESS;
}






void renderDeviceDestroyPipelines(RenderDevice device) {

	vkDestroyPipeline(device->device, device->renderPipeline.pipeline, nullptr);
	vkDestroyPipeline(device->device, device->fontPipeline.pipeline, nullptr);
	vkDestroyPipelineLayout(device->device, device->renderPipeline.layout, nullptr);
	vkDestroyPipelineLayout(device->device, device->fontPipeline.layout, nullptr);

}

void renderDeviceDestroyRenderResources(RenderDevice device) {
	Window window = device->window;

	destroyDeviceBuffer(device->resources.vertexBuffer);
	destroyDeviceBuffer(device->resources.indexBuffer);

	cleanupSwapChain(device);

	vkDestroyRenderPass(device->device, window->renderPass, nullptr);
	avLog(AV_DEBUG_DESTROY, "destroyed renderpass");

	for (uint i = 0; i < window->frameCount; i++) {
		frameDestroyResources(device, window, window->frames[i]);
	}

	vkDestroyCommandPool(device->device, device->commandPool, nullptr);
	avLog(AV_DEBUG_DESTROY, "destoyed command pool");

	avFree(window->frames);


}

void renderDeviceDestroy(RenderDevice device) {


	vkDestroyDevice(device->device, nullptr);
	avLog(AV_DEBUG_DESTROY, "destroyed render device");

	avFree(device);
}



