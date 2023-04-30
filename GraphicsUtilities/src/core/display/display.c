#include "display.h"
#include <GLFW/glfw3.h>
#include "../util/util.h"

typedef struct DisplaySurface_T {
	uint width;
	uint height;
	DisplayType type;
} DisplaySurface_T;

AvResult displaySurfaceInit(DisplaySurface* displaySurface) {

	glfwInit();

	*displaySurface = avAllocate(sizeof(DisplaySurface_T), 1, "allocating displaysurface handle");

	GLFWmonitor* monitor = glfwGetPrimaryMonitor();

	if (monitor == NULL) {
		return AV_NO_SUPPORT;
	}

	GLFWvidmode* mode = glfwGetVideoMode(monitor);

	(*displaySurface)->width = mode->width;
	(*displaySurface)->height = mode->height;
	(*displaySurface)->type = DISPLAY_TYPE_MONITOR;

	return AV_SUCCESS;
}

uint displaySurfaceGetWidth(DisplaySurface displaySurface) {
	return displaySurface->width;
}

uint displaySurfaceGetHeight(DisplaySurface displaySurface) {
	return displaySurface->height;
}

DisplayType displaySurfaceGetType(DisplaySurface displaySurface) {
	return displaySurface->type;
}

void displaySurfaceDeinit(DisplaySurface displaySurface) {
	glfwTerminate();

	avFree(displaySurface);
}

typedef struct Window_T {
	uint width;
	uint height;
	GLFWwindow* window;
} Window_T;

AvResult displaySurfaceCreateWindow(DisplaySurface displaySurface, WindowCreateInfo windowCreateInfo, WindowProperties* windowProperties, Window* window) {

	if (windowCreateInfo.width > displaySurface->width) {
		windowCreateInfo.width = displaySurface->width;
	}
	if (windowCreateInfo.height > displaySurface->height) {
		windowCreateInfo.height = displaySurface->height;
	}

	*window = avAllocate(sizeof(Window_T), 1, "allocating window handle");

	GLFWmonitor* fullscreen = windowCreateInfo.fullSurface ? glfwGetPrimaryMonitor() : NULL;

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, windowCreateInfo.resizable ? GLFW_TRUE : GLFW_FALSE);
	(*window)->window = glfwCreateWindow(windowCreateInfo.width, windowCreateInfo.height, windowCreateInfo.title, fullscreen, NULL);

	if (windowCreateInfo.x != -1 && windowCreateInfo.y != -1) {
		glfwSetWindowPos((*window)->window, windowCreateInfo.x, windowCreateInfo.y);
	} else if (windowCreateInfo.x == -1 || windowCreateInfo.y == -1) {
		avAssert(AV_UNUSUAL_ARGUMENTS, AV_SUCCESS, "single axis specified, should be both or none");
	}

	glfwShowWindow((*window)->window);

	glfwSetWindowCloseCallback((*window)->window, windowCreateInfo.onWindowDisconnect);
	if (windowCreateInfo.resizable) {
		if (!windowCreateInfo.onWindowResize) {
			avAssert(AV_UNSPECIFIED_CALLBACK, AV_SUCCESS, "window is resizable, but no resize callback is specified");
		} else {
			glfwSetWindowSizeCallback((*window)->window, windowCreateInfo.onWindowResize);
		}
	}

	uint width, height, x, y;
	glfwGetWindowSize((*window)->window, width, height);
	glfwGetWindowPos((*window)->window, x, y);

	windowProperties->fullSurface = windowCreateInfo.fullSurface;
	windowProperties->resizable = windowCreateInfo.resizable;
	windowProperties->title = windowCreateInfo.title;
	windowProperties->x = x;
	windowProperties->y = y;
	windowProperties->width = width;
	windowProperties->height = height;

	return AV_SUCCESS;
}

void displaySurfaceDestroyWindow(DisplaySurface displaySurface, Window window) {
	avFree(window);
}
