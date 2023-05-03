#pragma once
#include "../core.h"

#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_RESET   "\x1b[0m"

extern AvLogLevel AV_LOG_LEVEL;

void setLogSettings(AvLogSettings settings);
void setProjectDetails(const char* projectName, uint version);

typedef enum ValidationMessageType {
	VALIDATION_MESSAGE_TYPE_DEVICE_ADDRESS,
	VALIDATION_MESSAGE_TYPE_GENERAL,
	VALIDATION_MESSAGE_TYPE_VALIDATION,
	VALIDATION_MESSAGE_TYPE_PERFORMANCE,
}ValidationMessageType;

void logDeviceValidation(const char* renderer, AvLogLevel level, ValidationMessageType type, const char* message);