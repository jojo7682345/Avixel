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
	bool decorated;
} WindowProperties;

typedef struct WindowCreateInfo {
	WindowProperties properties;
	void (*onWindowResize)(AvWindow, uint, uint);
	void (*onWindowDisconnect)(AvWindow);
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
void windowUpdateEvents(Window window);
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


typedef enum DeviceStatus {
	DEVICE_STATUS_NORMAL = 0,
	DEVICE_STATUS_FATAL_ERROR = 1 << 0,
	DEVICE_STATUS_SHUTDOWN_REQUESTED = 1 << 1,
	DEVICE_STATUS_RESIZED = 1 << 2,
	DEVICE_STATUS_INOPERABLE = 1 << 3,
}DeviceStatus;


DeviceStatus renderInstanceGetStatus(RenderInstance instance);
DeviceStatus renderDeviceGetStatus(RenderDevice device);
DeviceStatus displaySurfaceGetStatus(DisplaySurface instance);
DeviceStatus windowGetStatus(Window window);

bool renderInstanceCheckValidationSupport();

// render device
typedef struct RenderDeviceCreateInfo {
	Window window;
} RenderDeviceCreateInfo;

typedef struct PipelineCreateInfo {

} PipelineCreateInfo;

void renderDeviceCreate(AvInstance instance, RenderDeviceCreateInfo createInfo, RenderDevice* device);
void renderDeviceCreateRenderResources(RenderDevice device);
void renderDeviceCreatePipelines(RenderDevice device, uint pipelineCount, PipelineCreateInfo* createInfos);

void renderDeviceWaitIdle(RenderDevice device);

typedef struct RenderCommandsInfo {

} RenderCommandsInfo;

AvResult renderDeviceAquireNextFrame(RenderDevice device);
AvResult renderDeviceRecordRenderCommands(RenderDevice device, RenderCommandsInfo commands);
AvResult renderDeviceRenderFrame(RenderDevice device);
AvResult renderDevicePresent(RenderDevice device);


void renderDeviceDestroyPipelines(RenderDevice device);
void renderDeviceDestroyRenderResources(RenderDevice device);
void renderDeviceDestroy(RenderDevice device);