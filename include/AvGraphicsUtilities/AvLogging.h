
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

#define AV_LOCATION_ARGS uint64 line, const char* file, const char* func
#define AV_LOCATION_PARAMS __LINE__, __FILE__,__func__

void avLog_(AvResult result, AV_LOCATION_ARGS, const char* msg);
void avAssert_(AvResult result, AvResult valid, AV_LOCATION_ARGS, const char* msg);

#ifdef DEBUG
#define avLog(result, message) avLog_(result,AV_LOCATION_PARAMS, message)
#define avAssert(result, valid, message) avAssert_(result,valid,AV_LOCATION_PARAMS,message)
#else 
#define avLog(result,message)
#define avAssert(result,valid,message)
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