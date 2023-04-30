#include "renderer.h"
#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>
#include "util/util.h"

#define MAX_FRAMES_IN_FLIGHT  2

const char* const requiredDefaultExtensions[] = {
	VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
};
const uint requiredDefaultExtensionCount = sizeof(requiredDefaultExtensions) / sizeof(const char*);

const char* const validationLayers[] = {
	"VK_LAYER_KHRONOS_validation",
};
const uint validationLayerCount = sizeof(validationLayers) / sizeof(const char*);


typedef struct Renderer_T {
	VkInstance instance;
	VkPhysicalDevice physicalDevice;
	VkDebugUtilsMessengerEXT* debugMessenger;
	VkDevice device;
	VkQueue graphicsQueue;
	VkQueue presentQueue;
}Renderer_T;

typedef struct Frame_T {
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
}Frame_T;

typedef struct Window_T {
	GLFWwindow* window;
	VkSurfaceKHR surface;

	VkExtent2D frameExtent;
	VkFormat frameFormat;

	VkCommandPool commandPool;

	Frame_T frames[MAX_FRAMES_IN_FLIGHT];
}Window_T;

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


