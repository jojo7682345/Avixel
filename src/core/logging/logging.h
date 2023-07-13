#pragma once
#include "../core.h"

extern const char* AV_COLOR_RED;
extern const char* AV_COLOR_GREEN;
extern const char* AV_COLOR_YELLOW  ;
extern const char* AV_COLOR_BLUE    ;
extern const char* AV_COLOR_MAGENTA ;
extern const char* AV_COLOR_CYAN    ;
extern const char* AV_COLOR_RESET   ;

extern AvLogLevel AV_LOG_LEVEL;


void setLogSettings(AvLogSettings settings);
void setProjectDetails(const char* projectName, uint version);

typedef enum ValidationMessageType {
	VALIDATION_MESSAGE_TYPE_DEVICE_ADDRESS,
	VALIDATION_MESSAGE_TYPE_GENERAL,
	VALIDATION_MESSAGE_TYPE_VALIDATION,
	VALIDATION_MESSAGE_TYPE_PERFORMANCE,
}ValidationMessageType;

void logDeviceValidation(const char* renderer, AvValidationLevel level, ValidationMessageType type, const char* message);