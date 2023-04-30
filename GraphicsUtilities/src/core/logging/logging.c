#include "logging.h"

AvLogLevel AV_LOG_LEVEL = AV_LOG_LEVEL_DEFAULT;
uint AV_LOG_LINE = AV_LOG_LINE_DEFAULT;
uint AV_LOG_FILE = AV_LOG_FILE_DEFAULT;
uint AV_LOG_FUNC = AV_LOG_FUNC_DEFAULT;
uint AV_LOG_PROJECT = AV_LOG_PROJECT_DEFAULT;
uint AV_LOG_TIME = AV_LOG_TIME_DEFAULT;
uint AV_LOG_TYPE = AV_LOG_TYPE_DEFAULT;
uint AV_LOG_ERROR = AV_LOG_ERROR_DEFAULT;
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
};

void setLogSettings(AvLogSettings settings) {
	AV_LOG_LEVEL = settings.level;
	AV_LOG_LINE = settings.printLine;
	AV_LOG_FILE = settings.printFile;
	AV_LOG_FUNC = settings.printFunc;
	AV_LOG_PROJECT = settings.printProject;
	AV_LOG_TIME = settings.printTime;
	AV_LOG_TYPE = settings.printType;
	AV_LOG_ERROR = settings.printError;
	AV_ASSERT_LEVEL = settings.assertLevel;
}

void setProjectDetails(const char* projectName, uint version) {

	strcpy_s((void*)AV_LOG_PROJECT_NAME, 63, projectName);
	AV_LOG_PROJECT_NAME[63] = '\0';

	AV_LOG_PROJECT_VERSION = version;
}

void printTags(AvResult result, AV_LOCATION_ARGS, const char* msg) {
	//message
	const char* message;

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
	default:
		message = "unknown";
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
		fprintf(fstream, "[%s]", time);
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
		fprintf(fstream, "[%s]", result_level);
	}
	if (AV_LOG_PROJECT) {
		fprintf(fstream, "[%s v%i.%i]", AV_LOG_PROJECT_NAME, AV_LOG_PROJECT_VERSION >> 16, AV_LOG_PROJECT_VERSION & 0xffff);
	}
	if (AV_LOG_FUNC) {
		fprintf(fstream, "[func: %s]", func);
	}
	if (AV_LOG_LINE) {
		fprintf(fstream, "[line %lu]", line);
	}
	if (AV_LOG_FILE) {
		fprintf(fstream, "[file: %s]", file);
	}
	if (AV_LOG_ERROR) {
		fprintf(fstream, " %s", message);
	}

	if (!(strcmp(msg, "") == 0)) {
		fprintf(fstream, " -> ");
	}
}


void avLog_(AvResult result, AV_LOCATION_ARGS, const char* msg) {

	if ((uint)result < AV_LOG_LEVEL) {
		return;
	}

	printTags(result, line, file, func, fstream, msg);

	fprintf(fstream, "%s\n", msg);

}

void avAssert_(AvResult result, AvResult valid, AV_LOCATION_ARGS, const char* msg) {
	if (result == valid && AV_ASSERT_LEVEL != AV_ASSERT_LEVEL_ALL) {
		return;
	}

	printTags(result, line, file, func, fstream, msg);
	fprintf(fstream, "assert -> ");
	fprintf(fstream, "%s\n", msg);

	if (result >= AV_ASSERT_LEVEL && result) {
		abort();
	}
}