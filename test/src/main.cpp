#define AV_LOG_CATEGORY "application"
#define DEBUG
#include <avixel/avixel.h>

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

const char* disabledLogCategories[] = {
	"avixel",
	//"avixel_core",
	//"avixel_renderer"
};
uint disabledLogCategoryCount = sizeof(disabledLogCategories) / sizeof(const char*);

AvResult disabledLogMessages[] = {
	//AV_DEBUG_CREATE,
	//AV_DEBUG_DESTROY,
};
uint disabledLogMessageCount = sizeof(disabledLogMessages) / sizeof(AvResult);

int main(int argC, const char** argV) {

	AvInstance instance{};

	AvLogSettings logSettings = avLogSettingsDefault;
	logSettings.printSuccess = true;
	logSettings.printCode = false;
	logSettings.printAssert = false;
	logSettings.printType = true;
	logSettings.printFunc = false;
	logSettings.printError = true;
	logSettings.printCategory = false;
	logSettings.colors = true;
	logSettings.disabledCategories = disabledLogCategories;
	logSettings.disabledCategoryCount = disabledLogCategoryCount;
	logSettings.disabledMessages = disabledLogMessages;
	logSettings.disabledMessageCount = disabledLogMessageCount;
	logSettings.validationLevel = AV_VALIDATION_LEVEL_WARNINGS_AND_ERRORS;
	logSettings.level = AV_LOG_LEVEL_ALL;

	AvWindowCreateInfo windowInfo = {};
	windowInfo.sType = AV_STRUCTURE_TYPE_WINDOW_CREATE_INFO;
	windowInfo.fullscreen = false;
	windowInfo.resizable = true;
	windowInfo.undecorated = false;
	windowInfo.width = 1280;
	windowInfo.height = 720;
	windowInfo.x = AV_WINDOW_POSITION_CENTERED;
	windowInfo.y = AV_WINDOW_POSITION_CENTERED;
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

	while (!avShutdownRequested(instance)) {

		avUpdate(instance);
	}

	avInstanceDestroy(instance);
	
	//PRINT_ALL_MEMORY_LEAKS();

	return 0;
}
