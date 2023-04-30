#pragma once
#include "../core.h"

typedef struct DisplaySurface_T* DisplaySurface;

typedef enum DisplayType {
	DISPLAY_TYPE_MONITOR,
	DISPLAY_TYPE_EMBEDDED
} DisplayType;

AvResult displaySurfaceInit(DisplaySurface* displaySurface);

uint displaySurfaceGetWidth(DisplaySurface displaySurface);

uint displaySurfaceGetHeight(DisplaySurface displaySurface);

DisplayType displaySurfaceGetType(DisplaySurface displaySurface);

void displaySurfaceDeinit(DisplaySurface displaySurface);

typedef struct Window_T* Window;

typedef struct WindowSizeProperties {
	uint width;
	uint height;	
	uint x;
	uint y;
}WindowSizeProperties;

typedef struct WindowProperties {
	WindowSizeProperties;
	const char* title;
	bool resizable;
	bool fullSurface;
} WindowProperties;

typedef struct WindowCreateInfo {
	WindowProperties;
	void (*onWindowResize)(void*, uint, uint);
	void (*onWindowDisconnect)(void*);
} WindowCreateInfo;

AvResult displaySurfaceCreateWindow(DisplaySurface displaySurface, WindowCreateInfo windowCreateInfo, WindowProperties* windowProperties, Window* window);
void displaySurfaceDestroyWindow(DisplaySurface displaySurface, Window window);
