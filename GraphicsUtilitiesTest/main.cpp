#include <GraphicsUtilities/GraphicsUtilities.h>
#define MU_DEBUG
#include <MemoryUtilities/MemoryUtilities.h>

int main(int argC, const char** argV) {

	AvInstance instance;
	AvWindow window;

	AvLogSettings logSettings = avLogSettingsDefault;
	logSettings.printSuccess = true;

	AvInstanceCreateInfo instanceInfo = {};
	instanceInfo.pProjectName = "GRAPHICS_UTILITIES_TEST";
	instanceInfo.projectVersion = AV_VERSION(1, 0);
	instanceInfo.logSettings = &logSettings;
	instanceInfo.disableVulkanValidation = false;

	avAssert(
		avInstanceCreate(instanceInfo, &instance), 
		AV_SUCCESS, 
		"instance creation"
	);

	AvWindowCreateInfo windowInfo = {};
	avAssert(
		avWindowCreate(instance, windowInfo, &window),
		AV_SUCCESS,
		"window creation"
	);

	avInstanceDestroy(instance);
	avWindowDestroy(window);
	
	PRINT_ALL_MEMORY_LEAKS();

	return 0;
}
