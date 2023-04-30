#include "core.h"
#include "GraphicsUtilities.h"


const char* const requiredDefaultExtensions[] = {
	VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
};
const uint requiredDefaultExtensionCount = sizeof(requiredDefaultExtensions) / sizeof(const char*);

const char* const validationLayers[] = {
	"VK_LAYER_KHRONOS_validation",
};
const uint validationLayerCount = sizeof(validationLayers) / sizeof(const char*);

void enumerateRequiredExtensions(uint* requiredExtensionCount, const char*** requiredExtensions) {
	uint glfwExtensionCount = 0;
	const char** glfwExtenstions;
	glfwExtenstions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

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

AvResult avInstanceCreate(AvInstanceCreateInfo createInfo, AvInstance* pInstance) {

	// glfw 
	glfwInit();

	// log configuration
	if (createInfo.logSettings) {
		setLogLevel(*createInfo.logSettings);
	}
	setProjectDetails(createInfo.pProjectName, createInfo.projectVersion);

	// validation assertion
	bool enableValidation = false;
	if (!createInfo.disableVulkanValidation && checkValidationLayerSupport()) {
		enableValidation = true;
		avAssert(AV_SUCCESS, AV_SUCCESS, "validationlayers present");
	} else if (!createInfo.disableVulkanValidation) {
		enableValidation = false;
		avAssert(AV_VALIDATION_NOT_PRESEND, VK_SUCCESS, "validationlayers requested not present");
	}

	// allocate instance handle;
	*pInstance = avAllocate(sizeof(AvInstance_T), 1, "allocating instance handle");

	// instance creation
	VkApplicationInfo appInfo = {0};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.apiVersion = VK_API_VERSION_1_0;
	appInfo.engineVersion = VK_MAKE_API_VERSION(0, 1, 0, 0);
	appInfo.pEngineName = "AlpineValleyUIengine";
	appInfo.pApplicationName = createInfo.pProjectName;
	appInfo.applicationVersion = createInfo.projectVersion;
	appInfo.pNext = nullptr;

	VkInstanceCreateInfo instanceInfo = { 0 };
	instanceInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	instanceInfo.pApplicationInfo = &appInfo;
	VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo = {0};
	if (enableValidation) {
		instanceInfo.enabledLayerCount = validationLayerCount;
		instanceInfo.ppEnabledLayerNames = validationLayers;
		populateDebugMessengerCreateInfo(&debugCreateInfo);
		instanceInfo.pNext = &debugCreateInfo;
	} else {
		instanceInfo.enabledLayerCount = 0;
	}

	uint requiredExtensionCount = 0;
	enumerateRequiredExtensions(&requiredExtensionCount, NULL);
	const char** requiredExtensions = avAllocate(sizeof(const char*), requiredExtensionCount, "failed to allocate memory for exension enumeration");
	enumerateRequiredExtensions(&requiredExtensionCount, &requiredExtensions);

	instanceInfo.enabledExtensionCount = requiredExtensionCount;
	instanceInfo.ppEnabledExtensionNames = requiredExtensions;

	VkResult result;
	result = vkCreateInstance(&instanceInfo, NULL, &(*pInstance)->instance);
	if (result != VK_SUCCESS) {
		avAssert(AV_CREATION_ERROR, AV_SUCCESS, "creating the vulkan instance");
	}
	avFree(requiredExtensions);

	// debug messenger setup
	if (!createInfo.disableVulkanValidation) {
		VkDebugUtilsMessengerCreateInfoEXT debugInfo;
		populateDebugMessengerCreateInfo(&debugInfo);
		(*pInstance)->debugMessenger = avAllocate(sizeof(VkDebugUtilsMessengerEXT), 1, "allocating memeory for debug messenger");
		result = createDebugUtilsMessengerEXT((*pInstance)->instance, &debugInfo, NULL, (*pInstance)->debugMessenger);
		if (result != VK_SUCCESS) {
			avAssert(AV_CREATION_ERROR, AV_SUCCESS, "creating the validation logger");
		}
	}

	// physical device selection
	uint32 deviceCount = 0;
	vkEnumeratePhysicalDevices((*pInstance)->instance, &deviceCount, NULL);
	if (deviceCount == 0) {
		avAssert(AV_NO_SUPPORT, AV_SUCCESS, "vulkan not supported on device");
	}
	VkPhysicalDevice* devices = avAllocate(sizeof(VkPhysicalDevice), deviceCount, "failed to allocate memory for gpu enumeration");
	vkEnumeratePhysicalDevices((*pInstance)->instance, &deviceCount, devices);

	uint bestScore = 0;
	VkPhysicalDevice bestDevice = VK_NULL_HANDLE;
	for (uint i = 0; i < deviceCount; i++) {
		VkPhysicalDevice device = devices[i];

		VkPhysicalDeviceProperties deviceProperties;
		vkGetPhysicalDeviceProperties(device, &deviceProperties);

		const char* deviceName = deviceProperties.deviceName;
		char msg[256 + (sizeof("found device ") / sizeof(char))] = { 0 };
		sprintf_s(msg, sizeof(msg), "found device %s", deviceName);
		avLog(AV_INFO, msg);

		uint score = scoreDevice(device);
		if (score > bestScore) {
			bestDevice = device;
			bestScore = score;
		}
	}
	avFree(devices);

	(*pInstance)->physicalDevice = bestDevice;
	if ((*pInstance)->physicalDevice == VK_NULL_HANDLE) {
		avAssert(AV_NO_SUPPORT, AV_SUCCESS, "no suitable gpu found");
	} else {
		VkPhysicalDeviceProperties deviceProperties;
		vkGetPhysicalDeviceProperties((*pInstance)->physicalDevice, &deviceProperties);
		const char* deviceName = deviceProperties.deviceName;
		char msg[256 + (sizeof("selected device ") / sizeof(char))] = { 0 };
		sprintf_s(msg, sizeof(msg), "selected device  %s", deviceName);
		avLog(AV_INFO, msg);
	}





	return AV_SUCCESS;
}

void avInstanceDestroy(AvInstance instance) {

	if (instance->debugMessenger) {
		destroyDebugUtilsMessengerEXT(instance->instance, *(instance->debugMessenger), NULL);
		avFree(instance->debugMessenger);
	}

	vkDestroyInstance(instance->instance,NULL);

	avFree(instance);

	glfwTerminate();
}

AvResult avWindowCreate(AvInstance instance, AvWindowCreateInfo createInfo, AvWindow* window) {
	return AV_SUCCESS;
}
void avWindowDestroy(AvWindow window) {

}
