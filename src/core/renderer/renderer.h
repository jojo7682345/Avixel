#pragma once
#include "../core.h"

typedef struct RenderInstance_T* RenderInstance;
typedef struct RenderDevice_T* RenderDevice;
typedef struct DisplaySurface_T* DisplaySurface;
typedef struct Window_T* Window;

typedef enum DisplayType {
	DISPLAY_TYPE_MONITOR,
	DISPLAY_TYPE_EMBEDDED
} DisplayType;

typedef enum RendererType {
	RENDERER_TYPE_VULKAN,
	RENDERER_TYPE_CUSTOM,
}RendererType;

typedef struct WindowSizeProperties{
    uint width;
    uint height;
    uint x;
    uint y;
} WindowSizeProperties;
typedef struct WindowProperties{
    WindowSizeProperties size;
	const char* title;
	bool resizable;
	bool fullSurface;
} WindowProperties;

typedef struct WindowCreateInfo {
	WindowProperties properties;
	void (*onWindowResize)(void*, int, int);
	void (*onWindowDisconnect)(void*);
} WindowCreateInfo;

RendererType getRendererType();

// surface
AvResult displaySurfaceInit(AvInstance instance);
uint displaySurfaceGetWidth(AvInstance instance);
uint displaySurfaceGetHeight(AvInstance instance);
DisplayType displaySurfaceGetType(AvInstance instance);
void displaySurfaceDeinit(AvInstance instance);
const char** displaySurfaceEnumerateExtensions(AvInstance instance, uint* count);

// window
AvResult displaySurfaceCreateWindow(AvInstance instance, WindowCreateInfo windowCreateInfo, WindowProperties* windowProperties, Window* window);
void displaySurfaceDestroyWindow(AvInstance instance, Window window);

// render instance
typedef struct RenderInstanceCreateInfo {
	AvProjectInfo projectInfo;
	bool deviceValidationEnabled;
	uint extensionCount;
	const char** extensions;
}RenderInstanceCreateInfo;

void renderInstanceCreate(AvInstance instance, RenderInstanceCreateInfo info);
void renderInstanceDestroy(AvInstance instance);
bool renderInstanceCheckValidationSupport();

// render device
typedef struct RenderDeviceCreateInfo {
	Window window;
} RenderDeviceCreateInfo;

typedef struct PipelineCreateInfo {

} PipelineCreateInfo;

void renderDeviceCreate(AvInstance instance, RenderDeviceCreateInfo createInfo, RenderDevice* device);
void renderDeviceCreateResources(RenderDevice device);
void renderDeviceCreatePipelines(RenderDevice device, uint pipelineCount, PipelineCreateInfo* createInfos);


void renderDeviceDestroyPipelines(RenderDevice device);
void renderDeviceDestroyResources(RenderDevice device);
void renderDeviceDestroy(RenderDevice device);