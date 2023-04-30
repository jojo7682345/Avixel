#pragma once
#include "../core.h"

#define MAX_FRAMES_IN_FLIGHT  2

typedef struct Renderer {
	VkInstance instance;
	VkPhysicalDevice physicalDevice;
	VkDebugUtilsMessengerEXT* debugMessenger;
	VkDevice device;
	VkQueue graphicsQueue;
	VkQueue presentQueue;
}Renderer;

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

typedef struct Window {
	GLFWwindow* window;
	VkSurfaceKHR surface;

	VkExtent2D frameExtent;
	VkFormat frameFormat;
	Frame frames[MAX_FRAMES_IN_FLIGHT];

}Window;

