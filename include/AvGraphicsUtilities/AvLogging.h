
typedef enum AvResult {
	// SUCCESS
	AV_SUCCESS = 0x00000000 | 0,
	AV_TEST_SUCCESS = AV_SUCCESS | 1,

	// DEBUG
	AV_DEBUG = 0x000F0000 | 0,
	AV_DEBUG_SUCCESS = AV_DEBUG | 1,
	AV_DEBUG_CREATE = AV_DEBUG | 2,
	AV_DEBUG_DESTROY = AV_DEBUG | 3,
	AV_DEBUG_INFO = AV_DEBUG | 4,
	AV_VALIDATION_PRESENT = AV_DEBUG | 5,
	AV_TEST_DEBUG = AV_DEBUG | 6,

	// INFO
	AV_INFO = 0x00F00000 | 0,
	AV_TEST_INFO = AV_INFO | 1,
	AV_TIME = AV_INFO | 2,

	// WARNING
	AV_WARNING = 0x0F000000 | 0,
	AV_OUT_OF_BOUNDS = AV_WARNING | 1,
	AV_VALIDATION_NOT_PRESENT = AV_WARNING | 2,
	AV_UNSPECIFIED_CALLBACK = AV_WARNING | 3,
	AV_UNUSUAL_ARGUMENTS = AV_WARNING | 4,
	AV_TEST_WARNING = AV_WARNING | 5,
	AV_TIMEOUT = AV_WARNING | 6,

	// ERROR
	AV_ERROR = 0xF0000000 | 0,
	AV_NO_SUPPORT = AV_ERROR | 1,
	AV_INVALID_ARGUMENTS = AV_ERROR | 2,
	AV_TIMED_OUT = AV_ERROR | 3,
	AV_MEMORY_ERROR = AV_ERROR | 4,
	AV_CREATION_ERROR = AV_ERROR | 5,
	AV_TEST_ERROR = AV_ERROR | 6,
	AV_IO_ERROR = AV_ERROR | 7,
	AV_NOT_FOUND = AV_ERROR | 8,
	AV_NOT_IMPLEMENTED = AV_ERROR | 9,
	AV_NOT_INITIALIZED = AV_ERROR | 10,
	AV_ALREADY_INITIALIZED = AV_ERROR | 11,
	AV_ALREADY_EXISTS = AV_ERROR | 12,
	AV_PARSE_ERROR = AV_ERROR | 13,
} AvResult;

typedef enum AvLogLevel {
	AV_LOG_LEVEL_ALL = AV_SUCCESS,
	AV_LOG_LEVEL_DEBUG = AV_DEBUG,
	AV_LOG_LEVEL_INFO = AV_INFO,
	AV_LOG_LEVEL_WARNING = AV_WARNING,
	AV_LOG_LEVEL_ERROR = AV_ERROR,
	AV_LOG_LEVEL_NONE = 0xFFFFFFFF,
} AvLogLevel;

typedef enum AvValidationLevel {
	AV_VALIDATION_LEVEL_VERBOSE = 0,
	AV_VALIDATION_LEVEL_INFO = 1,
	AV_VALIDATION_LEVEL_WARNINGS_AND_ERRORS = 2,
	AV_VALIDATION_LEVEL_ERRORS = 3,
}AvValidationLevel;

typedef enum AvAssertLevel {
	AV_ASSERT_LEVEL_NONE = 0,
	AV_ASSERT_LEVEL_PEDANTIC = AV_WARNING,
	AV_ASSERT_LEVEL_NORMAL = AV_ERROR,
}AvAssertLevel;

#define AV_LOG_LEVEL_DEFAULT AV_LOG_LEVEL_ALL
#define AV_LOG_LINE_DEFAULT 0
#define AV_LOG_FILE_DEFAULT 0
#define AV_LOG_FUNC_DEFAULT 0
#define AV_LOG_PROJECT_DEFAULT 0
#define AV_LOG_TIME_DEFAULT 0
#define AV_LOG_TYPE_DEFAULT 1
#define AV_LOG_ERROR_DEFAULT 1
#define AV_LOG_ASSERT_DEFAULT 0
#define AV_LOG_COLORS_DEFAULT 1
#define AV_ASSERT_LEVEL_DEFAULT AV_ASSERT_LEVEL_NORMAL
#define AV_LOG_CODE_DEFAULT 1
#define AV_VALIDATION_LEVEL_DEFAULT AV_VALIDATION_LEVEL_ERRORS
#define AV_LOG_CATEGORY_DEFAULT 0

extern const char* defaulDisabledLogCategories[];
extern const uint defaultDisabledLogCategoryCount;

extern const AvResult defaultDisabledMessages[];
extern const uint defaultDisabledCategoryCount;


#define AV_LOCATION_ARGS uint64 line, const char* file, const char* func
#define AV_LOCATION_PARAMS __LINE__, __FILE__,__func__
#define AV_CATEGORY_ARGS const char* category

void avLog_(AvResult result, AV_LOCATION_ARGS, AV_CATEGORY_ARGS, const char* msg);
void avAssert_(AvResult result, AvResult valid, AV_LOCATION_ARGS, AV_CATEGORY_ARGS, const char* msg);

#ifndef AV_LOG_CATEGORY
#define AV_LOG_CATEGORY "misc"
#endif

#ifdef DEBUG
#define avLog(result, message) avLog_(result,AV_LOCATION_PARAMS, AV_LOG_CATEGORY, message)
#define avAssert(result, valid, message) avAssert_(result,valid,AV_LOCATION_PARAMS, AV_LOG_CATEGORY, message)
#else 
#define avLog(result,message) (result)
#define avAssert(result,valid,message) (result)
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
	uint32 printAssert;
	uint32 printSuccess;
	uint32 printCategory;
	uint32 colors;
	AvAssertLevel assertLevel;
	uint32 printCode;
	AvValidationLevel validationLevel;

	uint32 disabledCategoryCount;
	const char** disabledCategories;

	uint32 disabledMessageCount;
	const AvResult* disabledMessages;
}AvLogSettings;
extern const AvLogSettings avLogSettingsDefault;