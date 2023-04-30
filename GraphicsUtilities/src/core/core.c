#include "core.h"
#include "GraphicsUtilities.h"


AvResult avInstanceCreate(AvInstanceCreateInfo createInfo, AvInstance* pInstance) {


	// log configuration
	if (createInfo.logSettings) {
		setLogSettings(*createInfo.logSettings);
	}
	setProjectDetails(createInfo.pProjectName, createInfo.projectVersion);

	// allocate instance handle;
	*pInstance = avAllocate(sizeof(AvInstance_T), 1, "allocating instance handle");

	displaySurfaceInit(&(*pInstance)->displaySurface);

	WindowCreateInfo windowInfo = { 0 };
	windowInfo.x = createInfo.windowInfo.x;
	windowInfo.y = createInfo.windowInfo.y;
	windowInfo.width = createInfo.windowInfo.width;
	windowInfo.height = createInfo.windowInfo.height;
	windowInfo.resizable = createInfo.windowInfo.resizable;
	windowInfo.fullSurface = createInfo.windowInfo.fullscreen;
	windowInfo.title = createInfo.windowInfo.title;
	windowInfo.onWindowDisconnect;


	WindowSizeProperties windowSize = { 0 };
	displaySurfaceCreateWindow((*pInstance)->displaySurface, windowInfo, &windowSize, (*pInstance)->window);

	//// validation assertion
	//bool enableValidation = false;
	//if (!createInfo.disableVulkanValidation && checkValidationLayerSupport()) {
	//	enableValidation = true;
	//	avAssert(AV_SUCCESS, AV_SUCCESS, "validationlayers present");
	//} else if (!createInfo.disableVulkanValidation) {
	//	enableValidation = false;
	//	avAssert(AV_VALIDATION_NOT_PRESEND, AV_SUCCESS, "validationlayers requested not present");
	//}

	//

	//

	//// instance creation
	//VkApplicationInfo appInfo = {0};
	//appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	//appInfo.apiVersion = VK_API_VERSION_1_0;
	//appInfo.engineVersion = VK_MAKE_API_VERSION(0, 1, 0, 0);
	//appInfo.pEngineName = "AlpineValleyUIengine";
	//appInfo.pApplicationName = createInfo.pProjectName;
	//appInfo.applicationVersion = createInfo.projectVersion;
	//appInfo.pNext = nullptr;

	//VkInstanceCreateInfo instanceInfo = { 0 };
	//instanceInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	//instanceInfo.pApplicationInfo = &appInfo;
	//VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo = {0};
	//if (enableValidation) {
	//	instanceInfo.enabledLayerCount = validationLayerCount;
	//	instanceInfo.ppEnabledLayerNames = validationLayers;
	//	populateDebugMessengerCreateInfo(&debugCreateInfo);
	//	instanceInfo.pNext = &debugCreateInfo;
	//} else {
	//	instanceInfo.enabledLayerCount = 0;
	//}

	//uint requiredExtensionCount = 0;
	//enumerateRequiredExtensions(&requiredExtensionCount, NULL);
	//const char** requiredExtensions = avAllocate(sizeof(const char*), requiredExtensionCount, "failed to allocate memory for exension enumeration");
	//enumerateRequiredExtensions(&requiredExtensionCount, &requiredExtensions);

	//instanceInfo.enabledExtensionCount = requiredExtensionCount;
	//instanceInfo.ppEnabledExtensionNames = requiredExtensions;

	//VkResult result;
	//result = vkCreateInstance(&instanceInfo, NULL, &(*pInstance)->instance);
	//if (result != VK_SUCCESS) {
	//	avAssert(AV_CREATION_ERROR, AV_SUCCESS, "creating the vulkan instance");
	//}
	//avFree(requiredExtensions);

	//// debug messenger setup
	//if (enableValidation) {
	//	VkDebugUtilsMessengerCreateInfoEXT debugInfo;
	//	populateDebugMessengerCreateInfo(&debugInfo);
	//	(*pInstance)->debugMessenger = avAllocate(sizeof(VkDebugUtilsMessengerEXT), 1, "allocating memeory for debug messenger");
	//	result = createDebugUtilsMessengerEXT((*pInstance)->instance, &debugInfo, NULL, (*pInstance)->debugMessenger);
	//	if (result != VK_SUCCESS) {
	//		avAssert(AV_CREATION_ERROR, AV_SUCCESS, "creating the validation logger");
	//	}
	//}

	//// physical device selection
	//uint32 deviceCount = 0;
	//vkEnumeratePhysicalDevices((*pInstance)->instance, &deviceCount, NULL);
	//if (deviceCount == 0) {
	//	avAssert(AV_NO_SUPPORT, AV_SUCCESS, "vulkan not supported on device");
	//}
	//VkPhysicalDevice* devices = avAllocate(sizeof(VkPhysicalDevice), deviceCount, "failed to allocate memory for gpu enumeration");
	//vkEnumeratePhysicalDevices((*pInstance)->instance, &deviceCount, devices);

	//uint bestScore = 0;
	//VkPhysicalDevice bestDevice = VK_NULL_HANDLE;
	//for (uint i = 0; i < deviceCount; i++) {
	//	VkPhysicalDevice device = devices[i];

	//	VkPhysicalDeviceProperties deviceProperties;
	//	vkGetPhysicalDeviceProperties(device, &deviceProperties);

	//	const char* deviceName = deviceProperties.deviceName;
	//	char msg[256 + (sizeof("found device ") / sizeof(char))] = { 0 };
	//	sprintf_s(msg, sizeof(msg), "found device %s", deviceName);
	//	avLog(AV_INFO, msg);

	//	uint score = scoreDevice(device);
	//	if (score > bestScore) {
	//		bestDevice = device;
	//		bestScore = score;
	//	}
	//}
	//avFree(devices);

	//(*pInstance)->physicalDevice = bestDevice;
	//if ((*pInstance)->physicalDevice == VK_NULL_HANDLE) {
	//	avAssert(AV_NO_SUPPORT, AV_SUCCESS, "no suitable gpu found");
	//} else {
	//	VkPhysicalDeviceProperties deviceProperties;
	//	vkGetPhysicalDeviceProperties((*pInstance)->physicalDevice, &deviceProperties);
	//	const char* deviceName = deviceProperties.deviceName;
	//	char msg[256 + (sizeof("selected device ") / sizeof(char))] = { 0 };
	//	sprintf_s(msg, sizeof(msg), "selected device  %s", deviceName);
	//	avLog(AV_INFO, msg);
	//}

	return AV_SUCCESS;
}

void avInstanceDestroy(AvInstance instance) {

	//if (instance->debugMessenger) {
	//	destroyDebugUtilsMessengerEXT(instance->instance, *(instance->debugMessenger), NULL);
	//	avFree(instance->debugMessenger);
	//}

	//vkDestroyInstance(instance->instance,NULL);

	displaySurfaceDeinit(instance->displaySurface);

	avFree(instance);


}

