#include "dynamicArray.h"

#include <memory.h>
#define INCREASE_AMOUNT 8

typedef struct DynamicArray_T {
	uint64 count;
	void* data;
	const uint dataSize;
	uint64 allocatedCount;
} DynamicArray_T;

void dynamicArrayCreate(uint dataSize, DynamicArray* dynamicArray) {

	// check if dataSize is equal to zero and give an error if this is the case
	if (dataSize == 0) {
		avAssert(AV_INVALID_SIZE, AV_SUCCESS, "specified data size must not be 0");
	}

	// allocate memory for dynamicArray 
	*dynamicArray = avAllocate(sizeof(DynamicArray_T), 1, "allocating dynamic array");

	// making instance of DynamicArray_T and setting value for dataSize
	DynamicArray_T base = { .dataSize = dataSize };

	// copy the base instance of the struct to the allocated memory in the heap
	memcpy(*dynamicArray, &base, sizeof(DynamicArray_T));

}

void dynamicArrayDestroy(DynamicArray dynamicArray) {

	// free the data
	avFree(dynamicArray->data);
	dynamicArray->data = nullptr;
	dynamicArray->count = 0;
	dynamicArray->allocatedCount = 0;

	// free the dynamicArray
	avFree(dynamicArray);

}

void dynamicArrayAdd(void* data, DynamicArray dynamicArray) {

	dynamicArrayAddRange(data, 1, dynamicArray);
}

void dynamicArrayAddRange(void* data, uint count, DynamicArray dynamicArray) {

	if (dynamicArray->count + count >= dynamicArray->allocatedCount) {
		// allocate more data
		dynamicArray->allocatedCount += count + INCREASE_AMOUNT;
		dynamicArray->data = avReallocate(dynamicArray->data, dynamicArray->dataSize, dynamicArray->allocatedCount, "increasing size of dynamic array");
	}

	memcpy((byte*)dynamicArray->data + (dynamicArray->count * dynamicArray->dataSize), data, dynamicArray->dataSize * count);
	dynamicArray->count += count;

}

uint dynamicArrayGetSize(DynamicArray dynamicArray) {
	uint dataSize = dynamicArray->count;
	return dataSize;
}

uint dynamicArrayGetAllocatedSize(DynamicArray dynamicArray) {
	uint allocatedMemory = dynamicArray->allocatedCount;
	return allocatedMemory;
}

uint64 dynamicArrayGetDataSize(DynamicArray dynamicArray) {
	uint64 dataSize = dynamicArray->dataSize;
	return dataSize;
}

void* dynamicArrayGetPtr(uint index, DynamicArray dynamicArray) {
	if (index >= dynamicArray->count) {
		avAssert(AV_OUT_OF_BOUNDS, 0, "accessing dynamic array out of bounds");
		return nullptr;
	}
	return (byte*)dynamicArray->data + index * dynamicArray->dataSize;
}

void dynamicArrayGet(void* data, uint index, DynamicArray dynamicArray) {

	void* ptr = dynamicArrayGetPtr(index, dynamicArray);
	
	if (!ptr) {
		avAssert(AV_MEMORY_ERROR, 0, "trying to write to invalid index");
		return;
	}

	memcpy(data, ptr, dynamicArray->dataSize);

}

void dynamicArraySet(void* data, uint index, DynamicArray dynamicArray) {

	void* ptr = dynamicArrayGetPtr(index, dynamicArray);

	if (!ptr) {
		avAssert(AV_MEMORY_ERROR, 0, "trying to write to invalid index");
		return;
	}

	memcpy(ptr, data, dynamicArray->dataSize);

}

void dynamicArrayClear(DynamicArray dynamicArray) {

	if (!dynamicArray->data) {
		return;
	}
	memset(dynamicArray->data, 0, dynamicArray->dataSize * dynamicArray->allocatedCount);
	dynamicArray->count = 0;
}

void dynamicArrayTrim(DynamicArray dynamicArray) {
	dynamicArray->data = avReallocate(dynamicArray->data, dynamicArray->dataSize, dynamicArray->count, "trimming dynamic array");
	dynamicArray->allocatedCount = dynamicArray->count;
}

void dynamicArrayFree(DynamicArray dynamicArray) {

	avFree(dynamicArray->data);
	dynamicArray->data = nullptr;
	dynamicArray->count = 0;
	dynamicArray->allocatedCount = 0;

}
