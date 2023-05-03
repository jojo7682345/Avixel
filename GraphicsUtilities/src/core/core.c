#include "core.h"
#include "GraphicsUtilities.h"


AvResult avInstanceCreate(AvInstanceCreateInfo createInfo, AvInstance* pInstance) {

	// log configuration
	if (createInfo.logSettings) {
		setLogSettings(*createInfo.logSettings);
	}
	setProjectDetails(createInfo.projectInfo.pProjectName, createInfo.projectInfo.projectVersion);

	// allocate instance handle;
	*pInstance = avAllocate(sizeof(AvInstance_T), 1, "allocating instance handle");

	// validation assertion
	bool enableValidation = false;
	if (!createInfo.disableDeviceValidation && renderInstanceCheckValidationSupport()) {
		enableValidation = true;
		avAssert(AV_SUCCESS, AV_SUCCESS, "validationlayers present");
	} else if (!createInfo.disableDeviceValidation) {
		enableValidation = false;
		avAssert(AV_VALIDATION_NOT_PRESEND, AV_SUCCESS, "validationlayers requested not present");
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
	windowInfo.x = createInfo.windowInfo.x;
	windowInfo.y = createInfo.windowInfo.y;
	windowInfo.width = createInfo.windowInfo.width;
	windowInfo.height = createInfo.windowInfo.height;
	windowInfo.resizable = createInfo.windowInfo.resizable;
	windowInfo.fullSurface = createInfo.windowInfo.fullscreen;
	windowInfo.title = createInfo.windowInfo.title;
	windowInfo.onWindowDisconnect;	// TODO: setup close event
	windowInfo.onWindowResize;		// TODO: setup resize event
	displaySurfaceCreateWindow(*pInstance, windowInfo, nullptr, &(*pInstance)->window);

	RenderDeviceCreateInfo renderDeviceInfo = { 0 };
	renderDeviceInfo.window = (*pInstance)->window;
	renderDeviceCreate(*pInstance, (*pInstance)->window, &(*pInstance)->renderDevice);

	renderDeviceCreateResources((*pInstance)->renderDevice, (*pInstance)->window);

	return AV_SUCCESS;
}

void avInstanceDestroy(AvInstance instance) {

	
	renderDeviceDestroyResources(instance->renderDevice, instance->window);

	renderDeviceDestroy(instance->renderDevice);

	displaySurfaceDeinit(instance);

	renderInstanceDestroy(instance);

	avFree(instance);


}

