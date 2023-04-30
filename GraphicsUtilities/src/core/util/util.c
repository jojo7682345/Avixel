#include "../core.h"
#include <stdlib.h>
#include <time.h>
#include <string.h>

#define MU_DEBUG
#include <MemoryUtilities/MemoryUtilities.h>

void* avAllocate_(uint typeSize, uint count, AV_LOCATION_ARGS, const char* errorMsg) {
	void* data = allocateHeapDebug((uint64)typeSize * (uint64)count,line,file);
	if (data == 0) {
		avAssert_(AV_MEMORY_ERROR, AV_SUCCESS, line, file, func, fstream, errorMsg);
		return NULL;
	}
	memset(data, 0, (uint64)typeSize * (uint64)count);
	return data;
}

void avFree_(void* data,AV_LOCATION_ARGS) {
	freeHeapDebug(data,line,file);
}




