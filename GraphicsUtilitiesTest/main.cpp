#include <GraphicsUtilities/GraphicsUtilities.h>
#define MU_DEBUG
#include <MemoryUtilities/MemoryUtilities.h>

int main(int argC, const char** argV) {

	AvInstance instance;

	AvLogSettings logSettings = avLogSettingsDefault;
	logSettings.printSuccess = true;

	AvWindowCreateInfo windowInfo = {};


	AvInstanceCreateInfo instanceInfo = {};
	instanceInfo.pProjectName = "GRAPHICS_UTILITIES_TEST";
	instanceInfo.projectVersion = AV_VERSION(1, 0);
	instanceInfo.logSettings = &logSettings;
	instanceInfo.disableVulkanValidation = false;
	instanceInfo.windowInfo = windowInfo;

	avAssert(
		avInstanceCreate(instanceInfo, &instance),
		AV_SUCCESS,
		"instance creation"
	);

	avInstanceDestroy(instance);
	
	PRINT_ALL_MEMORY_LEAKS();

	return 0;
}
