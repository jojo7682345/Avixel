
#include <GraphicsUtilities.h>

void buildInterface(AvInstance instance) {
	AvInterface interface;
	
	AvInterfaceLoadFileInfo loadInfo = { 0 };
	avInterfaceLoadFromFile(loadInfo, &interface, "./assets/testinterface.ui");
	
	//AvInterfaceLoadDataInfo dataInfo = { 0 };
	//avInterfaceLoadFromData(dataInfo, &interface, nullptr /*ptr to binary data*/, 0);


	//AvWindow mainWindow;
	//avInstanceGetPrimaryWindow(instance, &mainWindow);
	//avWindowSetInterface(mainWindow, instance, interface);

	
}


int main(int argC, const char** argV) {

	AvInstance instance;

	AvLogSettings logSettings = avLogSettingsDefault;
	logSettings.printSuccess = true;
	logSettings.printCode = false;
	logSettings.printType = true;
	logSettings.printFunc = false;
	logSettings.printError = true;
	logSettings.validationLevel = AV_LOG_LEVEL_WARNING;
	logSettings.assertLevel = AV_ASSERT_LEVEL_ALL;

	AvWindowCreateInfo windowInfo = {};
	windowInfo.sType = AV_STRUCTURE_TYPE_WINDOW_CREATE_INFO;
	windowInfo.fullscreen = false;
	windowInfo.resizable = false;
	windowInfo.width = 1280;
	windowInfo.height = 720;
	windowInfo.x = AV_WINDOW_POSITION_NOT_SPECIFIED;
	windowInfo.y = AV_WINDOW_POSITION_NOT_SPECIFIED;
	windowInfo.title = "window";

	AvProjectInfo projectInfo = {};
	projectInfo.pProjectName = "GRAPHICS_UTILITIES_TEST";
	projectInfo.projectVersion = AV_VERSION(1, 0);

	AvInstanceCreateInfo instanceInfo = {};
	instanceInfo.sType = AV_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	instanceInfo.projectInfo = projectInfo;
	instanceInfo.logSettings = &logSettings;
	instanceInfo.disableDeviceValidation = false;
	instanceInfo.windowInfo = windowInfo;

	avAssert(
		avInstanceCreate(instanceInfo, &instance),
		AV_SUCCESS,
		"instance creation"
	);


	buildInterface(instance);

	avInstanceDestroy(instance);
	
	//PRINT_ALL_MEMORY_LEAKS();

	return 0;
}
