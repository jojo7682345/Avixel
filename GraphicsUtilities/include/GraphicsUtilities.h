#ifndef __GRAPHICS_UTILITIES_INCLUDED__
#define __GRAPHICS_UTILITIES_INCLUDED__

#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef DISABLE_ALL_TYPEDEFS
	typedef unsigned int uint;
	typedef unsigned char byte;

#ifndef __cplusplus
#ifndef DISABLE_BOOL_TYPEDEF
	typedef unsigned char bool;
#define true 1
#define false 0
#endif
#ifndef DISABLE_NULLPTR_TYPEDEF
#define nullptr ((void*)0)
#endif
#endif

#ifndef DISABLE_TYPEDEFS
	typedef unsigned long long uint64;
	typedef unsigned int uint32;
	typedef unsigned short uint16;
	typedef unsigned char uint8;

	typedef unsigned long int64;
	typedef unsigned int int32;
	typedef unsigned short int16;
	typedef unsigned char int8;
#endif
#endif

	typedef struct Color {
		union {
			struct {
				byte r;
				byte g;
				byte b;
				byte a;
			};
			uint rgba;
		};
	} Color;

	typedef enum AvResult {
		// SUCCESS
		AV_SUCCESS = 0x00000000 | 0,

		// DEBUG
		AV_DEBUG = 0x000F0000 | 0,

		// INFO
		AV_INFO = 0x00F00000 | 0,

		// WARNING
		AV_WARNING = 0x0F000000 | 0,
		AV_OUT_OF_BOUNDS = AV_WARNING | 1,
		AV_VALIDATION_NOT_PRESEND = AV_WARNING | 2,
		AV_UNSPECIFIED_CALLBACK = AV_WARNING | 3,
		AV_UNUSUAL_ARGUMENTS = AV_WARNING | 4,

		// ERROR
		AV_ERROR = 0xF0000000 | 0,
		AV_NO_SUPPORT = AV_ERROR | 1,
		AV_INVALID_ARGUMENTS = AV_ERROR | 2,
		AV_TIMEOUT = AV_ERROR | 3,
		AV_MEMORY_ERROR = AV_ERROR | 4,
		AV_CREATION_ERROR  = AV_ERROR | 5,
	} AvResult;

	typedef enum AvLogLevel {
		AV_LOG_LEVEL_ALL = AV_SUCCESS,
		AV_LOG_LEVEL_DEBUG = AV_DEBUG,
		AV_LOG_LEVEL_INFO = AV_INFO,
		AV_LOG_LEVEL_WARNING = AV_WARNING,
		AV_LOG_LEVEL_ERROR = AV_ERROR,
		AV_LOG_LEVEL_NONE = 0xFFFFFFFF,
	} AvLogLevel;

	typedef enum AvAssertLevel {
		AV_ASSERT_LEVEL_ALL = 0,
		AV_ASSERT_LEVEL_WARNING = AV_WARNING,
		AV_ASSERT_LEVEL_ERROR = AV_ERROR,
		AV_ASSERT_LEVEL_NONE = 0xFFFFFFFF,
	}AvAssertLevel;

#define AV_LOG_LEVEL_DEFAULT AV_LOG_LEVEL_ALL
#define AV_LOG_LINE_DEFAULT 0
#define AV_LOG_FILE_DEFAULT 0
#define AV_LOG_FUNC_DEFAULT 0
#define AV_LOG_PROJECT_DEFAULT 0
#define AV_LOG_TIME_DEFAULT 0
#define AV_LOG_TYPE_DEFAULT 1
#define AV_LOG_ERROR_DEFAULT 1
#define AV_ASSERT_LEVEL_DEFAULT AV_ASSERT_LEVEL_ALL
#define AV_LOG_CODE_DEFAULT 1
#define AV_VALIDATION_LEVEL_DEFAULT AV_LOG_LEVEL_ALL

#define AV_LOCATION_ARGS uint64 line, const char* file, const char* func, FILE* fstream
#define AV_LOCATION_PARAMS __LINE__, __FILE__,__func__, stdout

	void avLog_(AvResult result, AV_LOCATION_ARGS, const char* msg);
#define avLog(result, message) avLog_(result,AV_LOCATION_PARAMS, message)

	void avAssert_(AvResult result, AvResult valid, AV_LOCATION_ARGS, const char* msg);
#define avAssert(result, valid, message) avAssert_(result,valid,AV_LOCATION_PARAMS,message)

	void* avAllocate_(uint size, uint count, AV_LOCATION_ARGS, const char* msg);
#define avAllocate(size,count,message) avAllocate_(size,count,AV_LOCATION_PARAMS, message)
	void avFree_(void* data, AV_LOCATION_ARGS);
#define avFree(data) avFree_(data, AV_LOCATION_PARAMS)

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

#ifdef __cplusplus
#define AV_DEFINE_STRUCT(object,...) typedef struct object : AvStructure { __VA_ARGS__ } object; 
#else
#define AV_DEFINE_STRUCT(object,...) typedef struct object { AvStructure; __VA_ARGS__ } object;  
#endif

	typedef struct AvLogSettings {
		AvLogLevel level;
		uint32 printLine;
		uint32 printFunc;
		uint32 printFile;
		uint32 printProject;
		uint32 printTime;
		uint32 printType;
		uint32 printError;
		uint32 printSuccess;
		AvAssertLevel assertLevel;
		uint32 printCode;
		AvLogLevel validationLevel;
	}AvLogSettings;
	extern const AvLogSettings avLogSettingsDefault;

	AV_DEFINE_STRUCT(AvWindowCreateInfo,
		uint x;
		uint y;
		uint width;
		uint height;
		const char* title;
		bool resizable;
		bool fullscreen;
	);
#define AV_WINDOW_POSITION_NOT_SPECIFIED (-1)

	// INSTANCE
	AV_DEFINE_HANDLE(AvInstance);
	AV_DEFINE_STRUCT(AvInstanceCreateInfo,
		AvProjectInfo projectInfo;
		AvLogSettings* logSettings;
		bool disableDeviceValidation;
		AvWindowCreateInfo windowInfo;
	);
	AvResult avInstanceCreate(AvInstanceCreateInfo createInfo, AvInstance* pInstance);
	void avInstanceDestroy(AvInstance instance);

#ifdef __cplusplus
}
#endif

#endif//__GRAPHICS_UTILITIES_INCLUDED__