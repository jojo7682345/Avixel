#include "logging.h"
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

AvLogLevel AV_LOG_LEVEL = AV_LOG_LEVEL_DEFAULT;
AvLogLevel AV_VALIDATION_LEVEL = AV_VALIDATION_LEVEL_DEFAULT;
uint AV_LOG_LINE = AV_LOG_LINE_DEFAULT;
uint AV_LOG_FILE = AV_LOG_FILE_DEFAULT;
uint AV_LOG_FUNC = AV_LOG_FUNC_DEFAULT;
uint AV_LOG_PROJECT = AV_LOG_PROJECT_DEFAULT;
uint AV_LOG_TIME = AV_LOG_TIME_DEFAULT;
uint AV_LOG_TYPE = AV_LOG_TYPE_DEFAULT;
uint AV_LOG_ERROR = AV_LOG_ERROR_DEFAULT;
uint AV_LOG_CODE = AV_LOG_CODE_DEFAULT;
AvAssertLevel AV_ASSERT_LEVEL = AV_ASSERT_LEVEL_DEFAULT;

char AV_LOG_PROJECT_NAME[64] = "PROJECT_NAME_NOT_SPECIFIED";
uint AV_LOG_PROJECT_VERSION = 0;

const AvLogSettings avLogSettingsDefault = {
	.level = AV_LOG_LEVEL_DEFAULT,
	.printLine = AV_LOG_LINE_DEFAULT,
	.printFile = AV_LOG_FILE_DEFAULT,
	.printFunc = AV_LOG_FUNC_DEFAULT,
	.printProject = AV_LOG_PROJECT_DEFAULT,
	.printTime = AV_LOG_TIME_DEFAULT,
	.printType = AV_LOG_TYPE_DEFAULT,
	.printError = AV_LOG_ERROR_DEFAULT,
	.assertLevel = AV_ASSERT_LEVEL_DEFAULT,
	.printCode = AV_LOG_CODE_DEFAULT,
	.validationLevel = AV_VALIDATION_LEVEL_DEFAULT
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
	AV_ASSERT_LEVEL = settings.assertLevel;
	AV_LOG_CODE = settings.printCode;
}

void setProjectDetails(const char* projectName, uint version) {

	if(strlen(projectName) >= 64){
		avAssert(AV_MEMORY_ERROR,0,"project name must be shorter than 63 characters");
	}
	strcpy((void*)AV_LOG_PROJECT_NAME, projectName);
	AV_LOG_PROJECT_NAME[63] = '\0';

	AV_LOG_PROJECT_VERSION = version;
}

void logDeviceValidation(const char* renderer, AvLogLevel level, ValidationMessageType type, const char* message) {
	if ((uint)AV_VALIDATION_LEVEL > level) {
		return;
	}

	fprintf(stdout, ANSI_COLOR_RESET"["ANSI_COLOR_CYAN"%s"ANSI_COLOR_RESET"]", renderer);

	switch (type) {
	case VALIDATION_MESSAGE_TYPE_DEVICE_ADDRESS:
		fprintf(stdout, ANSI_COLOR_RESET"["ANSI_COLOR_YELLOW"address"ANSI_COLOR_RESET"]");
		break;
	case VALIDATION_MESSAGE_TYPE_GENERAL:
		fprintf(stdout, ANSI_COLOR_RESET"["ANSI_COLOR_CYAN"general"ANSI_COLOR_RESET"]");
		break;
	case VALIDATION_MESSAGE_TYPE_VALIDATION:
		fprintf(stdout, ANSI_COLOR_RESET"["ANSI_COLOR_RED"validation"ANSI_COLOR_RESET"]");
		break;
	case VALIDATION_MESSAGE_TYPE_PERFORMANCE:
		fprintf(stdout, ANSI_COLOR_RESET"["ANSI_COLOR_BLUE"performance"ANSI_COLOR_RESET"]");
		break;
	}
	fprintf(stdout, " -> %s\n", message);
}

void printTags(AvResult result, AV_LOCATION_ARGS, const char* msg) {
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
	case AV_SUCCESS:
		message = "success";
		break;
	case AV_DEBUG:
		message = "debug";
		break;
	case AV_INFO:
		message = "info";
		break;
	case AV_WARNING:
		message = "warning";
		break;
	case AV_ERROR:
		message = "error";
		break;
	case AV_OUT_OF_BOUNDS:
		message = "out of bounds";
		break;
	case AV_MEMORY_ERROR:
		message = "memory error";
		break;
	case AV_CREATION_ERROR:
		message = "creation error";
		break;
	case AV_NO_SUPPORT:
		message = "no support";
		break;
	case AV_INVALID_ARGUMENTS:
		message = "invalid arguments";
		break;
	case AV_TIMEOUT:
		message = "timeout";
		break;
	case AV_VALIDATION_NOT_PRESEND:
		message = "validation not present";
		break;
	case AV_UNSPECIFIED_CALLBACK:
		message = "unspecified callback";
		break;
	case AV_TEST_ERROR:
		message = "test error";
		break;
	case AV_NOT_INITIALIZED:
		message = "not initialized";
		break;
	case AV_NOT_IMPLEMENTED:
		message = "not implemented";
		break;
	case AV_NOT_FOUND:
		message = "not found";
		break;
	case AV_ALREADY_INITIALIZED:
		message = "already initialized";
		break;
	case AV_ALREADY_EXISTS:
		message = "already exists";
		break;
	case AV_PARSE_ERROR:
		message = "parse error";
		break;
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
		if (result == AV_SUCCESS) {
			result_level = ANSI_COLOR_GREEN "SUCESS" ANSI_COLOR_RESET;
		} else if (result & AV_INFO) {
			result_level = ANSI_COLOR_RESET "INFO" ANSI_COLOR_RESET;
		} else if (result & AV_DEBUG) {
			result_level = ANSI_COLOR_BLUE "DEBUG" ANSI_COLOR_RESET;
		} else if (result & AV_WARNING) {
			result_level = ANSI_COLOR_YELLOW "WARNING" ANSI_COLOR_RESET;
		} else if (result & AV_ERROR) {
			result_level = ANSI_COLOR_RED"ERROR" ANSI_COLOR_RESET;
		} else {
			result_level = "UNKNOWN";
		}
		fprintf(stdout, "[%s]", result_level);
	}
	if (AV_LOG_CODE) {
		const char* color = "";
		switch (error_type) {
		case 'S':
			color = ANSI_COLOR_GREEN;
			break;
		case 'D':
			color = ANSI_COLOR_BLUE;
			break;
		case 'I':
			color = ANSI_COLOR_RESET;
			break;
		case 'W':
			color = ANSI_COLOR_YELLOW;
			break;
		case 'E':
			color = ANSI_COLOR_RED;
			break;
		}
		fprintf(stdout, "[%s%s"ANSI_COLOR_RESET"]", color, error_code);
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
	if (AV_LOG_ERROR && (!unknownCode || !AV_LOG_CODE)) {
		fprintf(stdout, " %s", message);
	}

	if (!(strcmp(msg, "") == 0)) {
		fprintf(stdout, " -> ");
	}
}


void avLog_(AvResult result, AV_LOCATION_ARGS, const char* msg) {

	if ((uint)result < (uint)AV_LOG_LEVEL) {
		return;
	}

	printTags(result, line, file, func, msg);

	fprintf(stdout, "%s\n", msg);

}

void avAssert_(AvResult result, AvResult valid, AV_LOCATION_ARGS, const char* msg) {
	if (result == valid && AV_ASSERT_LEVEL != AV_ASSERT_LEVEL_ALL) {
		return;
	}

	printTags(result, line, file, func, msg);
	fprintf(stdout, ANSI_COLOR_YELLOW"assert"ANSI_COLOR_RESET" -> ");
	fprintf(stdout, "%s\n", msg);

	if (result >= (uint)AV_ASSERT_LEVEL && result) {
		exit(-1);
	}
}