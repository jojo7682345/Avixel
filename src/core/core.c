#include "core.h"
#undef AV_LOG_CATEGORY
#define AV_LOG_CATEGORY "AvCore"

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
	windowInfo.onWindowDisconnect;	// TODO: setup close event
	windowInfo.onWindowResize;		// TODO: setup resize event
	displaySurfaceCreateWindow(*pInstance, windowInfo, nullptr, &(*pInstance)->window);

	RenderDeviceCreateInfo renderDeviceInfo = { 0 };
	renderDeviceInfo.window = (*pInstance)->window;
	renderDeviceCreate(*pInstance, renderDeviceInfo, &(*pInstance)->renderDevice);

	renderDeviceCreateResources((*pInstance)->renderDevice);


	renderDeviceCreatePipelines((*pInstance)->renderDevice,0,nullptr);

	return AV_SUCCESS;
}

void avInstanceDestroy(AvInstance instance) {

	renderDeviceDestroyPipelines(instance->renderDevice);
	
	renderDeviceDestroyResources(instance->renderDevice);

	renderDeviceDestroy(instance->renderDevice);

	displaySurfaceDeinit(instance);

	renderInstanceDestroy(instance);

	avFree(instance);


}

