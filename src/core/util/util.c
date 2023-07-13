#include "../core.h"
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <stdio.h>


void* avAllocate_(uint typeSize, uint count, AV_LOCATION_ARGS, AV_CATEGORY_ARGS, const char* errorMsg) {
	void* data = malloc(((size_t)typeSize * (size_t)count));
	if (data == 0) {
		avAssert_(AV_MEMORY_ERROR, AV_SUCCESS, line, file, func, category, errorMsg);
		return NULL;
	}
	memset(data, 0, ((uint64)typeSize * (uint64)count));
	return data;
}

void* avReallocate_(void* data, uint typeSize, uint count, AV_LOCATION_ARGS, const char* errorMsg) {
	void* ptr = realloc(data, ((size_t)typeSize * (size_t)count));
	if (ptr == 0) {
		avAssert_(AV_MEMORY_ERROR, AV_SUCCESS, line, file, func, errorMsg);
		return nullptr;
	}
	return ptr;
}

void avFree_(void* data,AV_LOCATION_ARGS) {
	free(data);
}

bool isDecNumber(char chr) {
	if (chr >= '0' && chr <= '9') {
		return true;
	}
	return false;
}

bool stringEquals(const char* strA, const char* strB, uint size) {
	for (uint i = 0; i < size; i++) {
		if (strA[i] != strB[i]) {
			return false;
		}
	}
	return true;
}

bool isBool(const char* str, uint size) {
	if (size == 4) {
		if (stringEquals(str, "true", 4)) {
			return true;
		}
		return false;
	}
	if (size == 5) {
		if (stringEquals(str, "false", 5)) {
			return true;
		}
		return false;
	}
	return false;
}

bool isParam(const char* str, uint size) {

	// export NAME = VALUE;
	// import NAME = VALUE;
	// define NAME = VALUE;

	const char export[] = "export";
	const char import[] = "import";
	const char define[] = "define";
	const uint paramSize = sizeof(export) / sizeof(char) - 1;

	if (size != paramSize) {
		return false;
	}

	if (stringEquals(str, export, paramSize)) {
		return true;
	}

	if (stringEquals(str, import, paramSize)) {
		return true;
	}

	if (stringEquals(str, define, paramSize)) {
		return true;
	}

	return false;

}

bool isHexNumber(char chr) {
	if (isDecNumber(chr)) {
		return true;
	}
	if (chr >= 'a' && chr <= 'f') {
		return true;
	}
	if (chr >= 'A' && chr <= 'F') {
		return true;
	}
	return false;
}

bool isLowerCaseLetter(char chr) {
	if (chr >= 'a' && chr <= 'z') {
		return true;
	}
	return false;
}

bool isUpperCaseLetter(char chr) {
	if (chr >= 'A' && chr <= 'Z') {
		return true;
	}
	return false;
}

bool isLetter(char chr) {
	if (isUpperCaseLetter(chr)) {
		return true;
	}
	if (isLowerCaseLetter(chr)) {
		return true;
	}
	return false;
}

bool isNameCharacter(char chr) {
	if (isUpperCaseLetter(chr)) {
		return true;
	}
	if (isLowerCaseLetter(chr)) {
		return true;
	}
	if (chr == '_') {
		return true;
	}
	if (isDecNumber(chr)) {
		return true;
	}
	return false;
}

bool isTextCharacter(char chr) {
	if (chr == '\n') {
		return false;
	}
	if (chr == '\r') {
		return false;
	}
	return true;
}

void skipToNextLine(const char* buffer, uint64 size, uint* i) {
	char c;
	while ((c = buffer[(*i)++]) != '\n' && *i <= size) {
		;;
	};
	(*i)--;
}




