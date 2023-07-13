#include "core.h"

#undef AV_LOG_CATEGORY
#define AV_LOG_CATEGORY "avixel_core"

AvResult avInstanceCreate(AvInstanceCreateInfo createInfo, AvInstance* pInstance) {

	// log configuration
	if (createInfo.logSettings) {
		setLogSettings(*createInfo.logSettings);
	}
	setProjectDetails(createInfo.projectInfo.pProjectName, createInfo.projectInfo.projectVersion);

	// allocate instance handle;
	*pInstance = avAllocate(sizeof(AvInstance_T), 1, "allocating instance handle");

	RendererType rendererType = getRendererType();
	switch (rendererType) {
	case RENDERER_TYPE_VULKAN:
		avLog(AV_DEBUG, "using vulkan renderer");
		break;
	case RENDERER_TYPE_CUSTOM:
		avLog(AV_DEBUG, "using custom renderer");
		break;
	}

	// validation assertion
	bool enableValidation = false;
	if (!createInfo.disableDeviceValidation && renderInstanceCheckValidationSupport()) {
		enableValidation = true;
		avAssert(AV_VALIDATION_PRESENT, AV_VALIDATION_PRESENT, "validationlayers present");
	} else if (!createInfo.disableDeviceValidation) {
		enableValidation = false;
		avAssert(AV_VALIDATION_NOT_PRESENT, AV_VALIDATION_PRESENT, "validationlayers requested not present");
	}

	// init displaySurface
	displaySurfaceInit(*pInstance);

	// render instance creation
	RenderInstanceCreateInfo renderInstanceInfo = { 0 };
	renderInstanceInfo.deviceValidationEnabled = enableValidation;
	renderInstanceInfo.extensions = displaySurfaceEnumerateExtensions(*pInstance, &renderInstanceInfo.extensionCount);
	renderInstanceInfo.projectInfo = createInfo.projectInfo;
	renderInstanceCreate(*pInstance, renderInstanceInfo);

	// create window
	WindowCreateInfo windowInfo = { 0 };
	windowInfo.properties.size.x = createInfo.windowInfo.x;
	windowInfo.properties.size.y = createInfo.windowInfo.y;
	windowInfo.properties.size.width = createInfo.windowInfo.width;
	windowInfo.properties.size.height = createInfo.windowInfo.height;
	windowInfo.properties.resizable = createInfo.windowInfo.resizable;
	windowInfo.properties.fullSurface = createInfo.windowInfo.fullscreen;
	windowInfo.properties.title = createInfo.windowInfo.title;
	windowInfo.properties.decorated = !createInfo.windowInfo.undecorated;
	windowInfo.onWindowDisconnect;	// TODO: setup close event
	windowInfo.onWindowResize;		// TODO: setup resize event
	displaySurfaceCreateWindow(*pInstance, windowInfo, nullptr, &(*pInstance)->window);

	RenderDeviceCreateInfo renderDeviceInfo = { 0 };
	renderDeviceInfo.window = (*pInstance)->window;
	renderDeviceCreate(*pInstance, renderDeviceInfo, &(*pInstance)->renderDevice);

	renderDeviceCreateRenderResources((*pInstance)->renderDevice);


	renderDeviceCreatePipelines((*pInstance)->renderDevice, 0, nullptr);

	return AV_SUCCESS;
}

void avInstanceDestroy(AvInstance instance) {

	renderDeviceWaitIdle(instance->renderDevice);

	renderDeviceDestroyPipelines(instance->renderDevice);

	renderDeviceDestroyRenderResources(instance->renderDevice);

	renderDeviceDestroy(instance->renderDevice);

	displaySurfaceDeinit(instance);

	renderInstanceDestroy(instance);

	avFree(instance);


}

void avUpdate(AvInstance instance) {

	if (!(windowGetStatus(instance->window) & DEVICE_STATUS_INOPERABLE)) {
		RenderCommandsInfo commandsInfo = {};
		renderDeviceAquireNextFrame(instance->renderDevice);
		renderDeviceRecordRenderCommands(instance->renderDevice, commandsInfo);
		renderDeviceRenderFrame(instance->renderDevice);
		renderDevicePresent(instance->renderDevice);
	}
	windowUpdateEvents(instance->window);
}

bool avShutdownRequested(AvInstance instance) {

	uint status = renderInstanceGetStatus(instance->renderInstance) |
		displaySurfaceGetStatus(instance->displaySurface) |
		renderDeviceGetStatus(instance->renderDevice) |
		windowGetStatus(instance->window);
	if (status & DEVICE_STATUS_FATAL_ERROR) {
		avLog(AV_SHUTDOWN_REQUESTED, "fatal error, resulting in shutdown request");
		return true;
	}

	if (status & DEVICE_STATUS_SHUTDOWN_REQUESTED) {
		avLog(AV_SHUTDOWN_REQUESTED, "window requested shutdown");
		return true;
	}

	return false;
}

