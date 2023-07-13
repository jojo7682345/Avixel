
#define AV_VERSION(major, minor) (major << 16 | minor)

typedef enum AvStructureType {
	AV_STRUCTURE_TYPE_INSTANCE_CREATE_INFO = 0,
	AV_STRUCTURE_TYPE_WINDOW_CREATE_INFO = 0
}AvStructureType;

#ifndef __cplusplus
typedef struct AvStructure AvStructure;
#endif	
typedef struct AvStructure {
	AvStructureType sType;
	AvStructure* pNext;
} AvStructure;

typedef struct AvProjectInfo {
	const char* pProjectName;
	uint projectVersion;
} AvProjectInfo;

#define AV_DEFINE_HANDLE(object) typedef struct object##_T* object;

// #ifdef __cplusplus
// #define AV_DEFINE_STRUCT(object,...) typedef struct object : AvStructure { __VA_ARGS__ } object; 
// #else
// #define AV_DEFINE_STRUCT(object,...) typedef struct object { AvStructure; __VA_ARGS__ } object;  
// #endif



typedef struct AvWindowCreateInfo{
	AvStructureType sType;
	void* next;
	uint x;
	uint y;
	uint width;
	uint height;
	const char* title;
	bool resizable;
	bool fullscreen;
	bool undecorated;
}AvWindowCreateInfo;
#define AV_WINDOW_POSITION_NOT_SPECIFIED (-1)
#define AV_WINDOW_POSITION_CENTERED (-2)

// INSTANCE
AV_DEFINE_HANDLE(AvInstance);
typedef struct AvInstanceCreateInfo{
	AvStructureType sType;
	void* next;
	AvProjectInfo projectInfo;
	AvLogSettings* logSettings;
	bool disableDeviceValidation;
	AvWindowCreateInfo windowInfo;

}AvInstanceCreateInfo;
AvResult avInstanceCreate(AvInstanceCreateInfo createInfo, AvInstance* pInstance);
void avInstanceDestroy(AvInstance instance);

void avUpdate(AvInstance instance);
bool avShutdownRequested(AvInstance instance);


AV_DEFINE_HANDLE(AvWindow);
void avInstanceGetPrimaryWindow(AvInstance, AvWindow* window);

AV_DEFINE_HANDLE(AvInterface);

