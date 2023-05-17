#include "dynamicArray.h"
#include "../core.h"

typedef struct DynamicArray_T {
	uint64 count;
	void* data;
	const uint dataSize;
} DynamicArray_T;

