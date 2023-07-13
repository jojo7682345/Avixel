#include "logging.h"
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>

#define ANSI_COLOR_RED		"\x1b[31m"
#define ANSI_COLOR_GREEN	"\x1b[32m"
#define ANSI_COLOR_YELLOW	"\x1b[33m"
#define ANSI_COLOR_BLUE		"\x1b[34m"
#define ANSI_COLOR_MAGENTA	"\x1b[35m"
#define ANSI_COLOR_CYAN		"\x1b[36m"
#define ANSI_COLOR_WHITE	"\x1b[36m"
#define ANSI_COLOR_RESET	"\x1b[00m"

#define ANSI_CARRAGE_RETURN "\x0d"

const char* AV_COLOR_RED = ANSI_COLOR_RED;
const char* AV_COLOR_GREEN = ANSI_COLOR_GREEN;
const char* AV_COLOR_YELLOW = ANSI_COLOR_YELLOW;
const char* AV_COLOR_BLUE = ANSI_COLOR_BLUE;
const char* AV_COLOR_MAGENTA = ANSI_COLOR_MAGENTA;
const char* AV_COLOR_CYAN = ANSI_COLOR_CYAN;
const char* AV_COLOR_WHITE = ANSI_COLOR_WHITE;
const char* AV_COLOR_RESET = ANSI_COLOR_RESET;

#define COLOR "%s"

AvLogLevel AV_LOG_LEVEL = AV_LOG_LEVEL_DEFAULT;
AvLogLevel AV_VALIDATION_LEVEL = AV_VALIDATION_LEVEL_DEFAULT;
uint AV_LOG_LINE = AV_LOG_LINE_DEFAULT;
uint AV_LOG_FILE = AV_LOG_FILE_DEFAULT;
uint AV_LOG_FUNC = AV_LOG_FUNC_DEFAULT;
uint AV_LOG_PROJECT = AV_LOG_PROJECT_DEFAULT;
uint AV_LOG_TIME = AV_LOG_TIME_DEFAULT;
uint AV_LOG_TYPE = AV_LOG_TYPE_DEFAULT;
uint AV_LOG_ERROR = AV_LOG_ERROR_DEFAULT;
uint AV_LOG_COLORS = AV_LOG_COLORS_DEFAULT;
uint AV_LOG_ASSERT = AV_LOG_ASSERT_DEFAULT;
uint AV_LOG_CODE = AV_LOG_CODE_DEFAULT;
uint AV_LOG_CATEGORY_ = AV_LOG_CATEGORY_DEFAULT;
uint blacklistedCategoryCount = 0;
const char** blacklistedCategories = nullptr;
AvAssertLevel AV_ASSERT_LEVEL = AV_ASSERT_LEVEL_DEFAULT;

uint blacklistedMessageCount = 0;
const AvResult* blacklistedMessages = nullptr;


char AV_LOG_PROJECT_NAME[64] = "PROJECT_NAME_NOT_SPECIFIED";
uint AV_LOG_PROJECT_VERSION = 0;

const char* defaulDisabledLogCategories[] = {
	"avixel",
	"avixel_core",
	"avixel_renderer"
};
const uint defaultDisabledLogCategoryCount = sizeof(defaulDisabledLogCategories) / sizeof(const char*);

const AvResult defaultDisabledMessages[] = {
};
const uint defaultDisabledMessageCount = sizeof(defaultDisabledMessages) / sizeof(AvResult);

const AvLogSettings avLogSettingsDefault = {
	.level = AV_LOG_LEVEL_DEFAULT,
	.printLine = AV_LOG_LINE_DEFAULT,
	.printFile = AV_LOG_FILE_DEFAULT,
	.printFunc = AV_LOG_FUNC_DEFAULT,
	.printProject = AV_LOG_PROJECT_DEFAULT,
	.printTime = AV_LOG_TIME_DEFAULT,
	.printType = AV_LOG_TYPE_DEFAULT,
	.printError = AV_LOG_ERROR_DEFAULT,
	.printAssert = AV_LOG_ASSERT_DEFAULT,
	.colors = AV_LOG_COLORS_DEFAULT,
	.assertLevel = AV_ASSERT_LEVEL_DEFAULT,
	.printCode = AV_LOG_CODE_DEFAULT,
	.validationLevel = AV_VALIDATION_LEVEL_DEFAULT,
	.printCategory = AV_LOG_CATEGORY_DEFAULT,
	.disabledCategoryCount = defaultDisabledLogCategoryCount,
	.disabledCategories = defaulDisabledLogCategories,
	.disabledMessageCount = defaultDisabledMessageCount,
	.disabledMessages = defaultDisabledMessages,
};

void setLogSettings(AvLogSettings settings) {
	AV_LOG_LEVEL = settings.level;
	AV_VALIDATION_LEVEL = settings.validationLevel;
	AV_LOG_LINE = settings.printLine;
	AV_LOG_FILE = settings.printFile;
	AV_LOG_FUNC = settings.printFunc;
	AV_LOG_PROJECT = settings.printProject;
	AV_LOG_TIME = settings.printTime;
	AV_LOG_TYPE = settings.printType;
	AV_LOG_ERROR = settings.printError;
	AV_LOG_ASSERT = settings.printAssert;
	AV_LOG_COLORS = settings.colors;
	AV_ASSERT_LEVEL = settings.assertLevel;
	AV_LOG_CODE = settings.printCode;
	AV_LOG_CATEGORY_ = settings.printCategory;

	if (settings.colors) {
		AV_COLOR_RED = ANSI_COLOR_RED;
		AV_COLOR_GREEN = ANSI_COLOR_GREEN;
		AV_COLOR_YELLOW = ANSI_COLOR_YELLOW;
		AV_COLOR_BLUE = ANSI_COLOR_BLUE;
		AV_COLOR_MAGENTA = ANSI_COLOR_MAGENTA;
		AV_COLOR_CYAN = ANSI_COLOR_CYAN;
		AV_COLOR_CYAN = ANSI_COLOR_WHITE;
		AV_COLOR_RESET = ANSI_COLOR_RESET;
	} else {
		AV_COLOR_RED = "";
		AV_COLOR_GREEN = "";
		AV_COLOR_YELLOW = "";
		AV_COLOR_BLUE = "";
		AV_COLOR_MAGENTA = "";
		AV_COLOR_CYAN = "";
		AV_COLOR_WHITE = "";
		AV_COLOR_RESET = "";
	}

	blacklistedCategories = settings.disabledCategories;
	blacklistedCategoryCount = settings.disabledCategoryCount;
	blacklistedMessages = settings.disabledMessages;
	blacklistedMessageCount = settings.disabledMessageCount;

}

bool checkCategoryDisabled(const char* category) {
	for (uint i = 0; i < blacklistedCategoryCount; i++) {
		if (strcmp(blacklistedCategories[i], category) == 0) {
			return true;
		}
	}
	return false;
}

bool checkMessageDisabled(AvResult result) {
	for (uint i = 0; i < blacklistedMessageCount; i++) {
		if (blacklistedMessages[i] == result){
			return true;
		}
	}
	return false;
}

void setProjectDetails(const char* projectName, uint version) {

	if (strlen(projectName) >= 64) {
		avAssert(AV_MEMORY_ERROR, 0, "project name must be shorter than 63 characters");
	}
	strcpy((void*)AV_LOG_PROJECT_NAME, projectName);
	AV_LOG_PROJECT_NAME[63] = '\0';

	AV_LOG_PROJECT_VERSION = version;
}

void logDeviceValidation(const char* renderer, AvValidationLevel level, ValidationMessageType type, const char* message) {
	if (AV_VALIDATION_LEVEL > level) {
		return;
	}

	fprintf(stdout, COLOR"["COLOR"%s"COLOR"]", AV_COLOR_RESET, AV_COLOR_CYAN, renderer, AV_COLOR_RESET);

	switch (type) {
	case VALIDATION_MESSAGE_TYPE_DEVICE_ADDRESS:
		fprintf(stdout, COLOR"["COLOR"address"COLOR"]", AV_COLOR_RESET, AV_COLOR_YELLOW, AV_COLOR_RESET);
		break;
	case VALIDATION_MESSAGE_TYPE_GENERAL:
		fprintf(stdout, COLOR"["COLOR"general"COLOR"]", AV_COLOR_RESET, AV_COLOR_CYAN, AV_COLOR_RESET);
		break;
	case VALIDATION_MESSAGE_TYPE_VALIDATION:
		fprintf(stdout, COLOR"["COLOR"validation"COLOR"]", AV_COLOR_RESET, AV_COLOR_RED, AV_COLOR_RESET);
		break;
	case VALIDATION_MESSAGE_TYPE_PERFORMANCE:
		fprintf(stdout, COLOR"["COLOR"performance"COLOR"]", AV_COLOR_RESET, AV_COLOR_BLUE, AV_COLOR_RESET);
		break;
	}
	fprintf(stdout, " -> %s\n", message);
}

#define MESSAGE(code,msg) case code: message = msg; break

void printTags(AvResult result, AV_LOCATION_ARGS, AV_CATEGORY_ARGS, const char* msg) {
	//message
	const char* message;

	char error_code[12] = { 0 };
	char error_type = 'S';
	uint error_num = result;
	if ((result & AV_DEBUG) == AV_DEBUG) {
		error_type = 'D';
		error_num = result & (~AV_DEBUG);
	}
	if ((result & AV_INFO) == AV_INFO) {
		error_type = 'I';
		error_num = result & (~AV_INFO);
	}
	if ((result & AV_WARNING) == AV_WARNING) {
		error_type = 'W';
		error_num = result & (~AV_WARNING);
	}
	if ((result & AV_ERROR) == AV_ERROR) {
		error_type = 'E';
		error_num = result & (~AV_ERROR);
	}
	sprintf(error_code, "%c%i", error_type, error_num);

	bool unknownCode = false;

	switch (result) {
		// SUCCESS
		MESSAGE(AV_SUCCESS, "success");	// 0
		MESSAGE(AV_TEST_SUCCESS, "test success"); // 1

		// DEBUG
		MESSAGE(AV_DEBUG, "debug"); // 0
		MESSAGE(AV_DEBUG_CREATE, "create"); // 1
		MESSAGE(AV_DEBUG_DESTROY, "destroy"); // 2
		MESSAGE(AV_DEBUG_SUCCESS, "success"); // 3
		MESSAGE(AV_DEBUG_INFO, "info"); // 4
		MESSAGE(AV_VALIDATION_PRESENT, "validation"); // 5
		MESSAGE(AV_TEST_DEBUG, "test debug"); // 6
		MESSAGE(AV_SHUTDOWN_REQUESTED, "shutdown requested"); // 7
		MESSAGE(AV_SWAPCHAIN_RECREATION, "swapchain recreation"); // 8
		MESSAGE(AV_WINDOW_SIZE, "window size"); // 9

		// INFO
		MESSAGE(AV_INFO, "info"); // 0 
		MESSAGE(AV_TEST_INFO, "test info"); // 1
		MESSAGE(AV_TIME, "time"); // 2

		// WARNING
		MESSAGE(AV_WARNING, "warning"); // 0
		MESSAGE(AV_OUT_OF_BOUNDS, "out of bounds"); // 1
		MESSAGE(AV_VALIDATION_NOT_PRESENT, "validation not present"); // 2
		MESSAGE(AV_UNSPECIFIED_CALLBACK, "unspecified callback"); // 3
		MESSAGE(AV_UNUSUAL_ARGUMENTS, "unusual arguments"); // 4
		MESSAGE(AV_TEST_WARNING, "test warning"); // 5
		MESSAGE(AV_TIMEOUT, "timeout"); // 6

		// ERROR
		MESSAGE(AV_ERROR, "error"); // 0
		MESSAGE(AV_NO_SUPPORT, "no support"); // 1
		MESSAGE(AV_INVALID_ARGUMENTS, "invalid arguments"); // 2
		MESSAGE(AV_TIMED_OUT, "timed out"); // 3
		MESSAGE(AV_MEMORY_ERROR, "memory error"); // 4
		MESSAGE(AV_CREATION_ERROR, "creation error"); // 5
		MESSAGE(AV_TEST_ERROR, "test error"); // 6
		MESSAGE(AV_IO_ERROR, "I/O error"); // 7
		MESSAGE(AV_NOT_FOUND, "not found"); // 8
		MESSAGE(AV_NOT_IMPLEMENTED, "not implemented"); // 9
		MESSAGE(AV_NOT_INITIALIZED, "not initialized"); // 10
		MESSAGE(AV_ALREADY_INITIALIZED, "already initialized"); // 11
		MESSAGE(AV_ALREADY_EXISTS, "already exists"); // 12
		MESSAGE(AV_PARSE_ERROR, "parse error"); // 13
		MESSAGE(AV_RENDER_COMMAND_ERROR, "render command error"); // 14
		MESSAGE(AV_RENDER_ERROR, "render error"); // 15
		MESSAGE(AV_PRESENT_ERROR, "present error"); // 16
		MESSAGE(AV_SWAPCHAIN_ERROR, "swapchain error"); // 17
	default:
		message = error_code;
		unknownCode = true;
		break;
	}

	if (AV_LOG_TIME) {
		//time 
		time_t rawtime;
		struct tm* timeinfo;
		time(&rawtime);
		timeinfo = localtime(&rawtime);
		char time[80];
		strftime(time, 80, "%T", timeinfo);
		fprintf(stdout, "[%s]", time);
	}
	if (AV_LOG_TYPE) {
		//level
		const char* result_level;
		const char* color;
		if (result == AV_SUCCESS) {
			result_level = "SUCESS";
			color = AV_COLOR_GREEN;
		} else if (result & AV_INFO) {
			result_level = "INFO";
			color = AV_COLOR_RESET;
		} else if (result & AV_DEBUG) {
			result_level = "DEBUG";
			color = AV_COLOR_BLUE;
		} else if (result & AV_WARNING) {
			result_level = "WARNING";
			color = AV_COLOR_YELLOW;
		} else if (result & AV_ERROR) {
			result_level = "ERROR";
			color = AV_COLOR_RED;
		} else {
			result_level = "UNKNOWN";
			color = AV_COLOR_WHITE;
		}
		fprintf(stdout, "["COLOR"%s"COLOR"]", color, result_level, AV_COLOR_RESET);
	}
	if (AV_LOG_CODE) {
		const char* color = "";
		switch (error_type) {
		case 'S':
			color = AV_COLOR_GREEN;
			break;
		case 'D':
			color = AV_COLOR_BLUE;
			break;
		case 'I':
			color = AV_COLOR_RESET;
			break;
		case 'W':
			color = AV_COLOR_YELLOW;
			break;
		case 'E':
			color = AV_COLOR_RED;
			break;
		}
		fprintf(stdout, "[%s%s"COLOR"]", color, error_code, AV_COLOR_RESET);
	}
	if (AV_LOG_PROJECT) {
		fprintf(stdout, "[%s v%i.%i]", AV_LOG_PROJECT_NAME, AV_LOG_PROJECT_VERSION >> 16, AV_LOG_PROJECT_VERSION & 0xffff);
	}
	if (AV_LOG_FUNC) {
		fprintf(stdout, "[func: %s]", func);
	}
	if (AV_LOG_LINE) {
		fprintf(stdout, "[line %llu]", line);
	}
	if (AV_LOG_FILE) {
		fprintf(stdout, "[file: %s]", file);
	}
	if (AV_LOG_CATEGORY_) {
		fprintf(stdout, "[category: %s]", category);
	}
	if (AV_LOG_ERROR && (!unknownCode || !AV_LOG_CODE)) {
		fprintf(stdout, " %s", message);
	}

	if (!(strcmp(msg, "") == 0)) {
		fprintf(stdout, " -> ");
	}
}

void avLog_(AvResult result, AV_LOCATION_ARGS, AV_CATEGORY_ARGS, const char* msg) {

	if (checkCategoryDisabled(category) || checkMessageDisabled(result)) {
		return;
	}
	
	if ((uint)result < (uint)AV_LOG_LEVEL) {
		return;
	}

	printTags(result, line, file, func, category, msg);

	fprintf(stdout, "%s\n", msg);

}

void avAssert_(AvResult result, AvResult valid, AV_LOCATION_ARGS, AV_CATEGORY_ARGS, const char* msg) {

	bool enabled = !checkCategoryDisabled(category) && !checkMessageDisabled(result);

	if ((result >= AV_LOG_LEVEL || result >= AV_ASSERT_LEVEL) && enabled) {
		printTags(result, line, file, func, category, msg);
		if (AV_LOG_ASSERT) {
			fprintf(stdout, COLOR"assert"COLOR" -> ", AV_COLOR_YELLOW, AV_COLOR_RESET);
		}
		fprintf(stdout, "%s\n", msg);
	}

	if (result == valid && result < AV_ASSERT_LEVEL) {
		return;
	}

	if (result >= AV_ASSERT_LEVEL) {
		exit(-1);
	}
}