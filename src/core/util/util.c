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

void avFree_(void* data,AV_LOCATION_ARGS) {
	free(data);
}




