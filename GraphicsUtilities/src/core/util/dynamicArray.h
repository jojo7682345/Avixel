#pragma once
#include "../core.h"

typedef struct DynamicArray_T* DynamicArray;

void dynamicArrayCreate(uint dataSize, DynamicArray* dynamicArray);
void dynamicArrayDestroy(DynamicArray dynamicArray);

void dynamicArrayAdd(void* data, DynamicArray dynamicArray);
void dynamicArrayAddRange(void* data, uint count, DynamicArray dynamicArray);

uint dynamicArrayGetSize(DynamicArray dynamicArray);
uint dynamicArrayGetAllocatedSize(DynamicArray dynamicArray);
uint64 dynamicArrayGetDataSize(DynamicArray dynamicArray);

/// <summary>
/// WARNING: after use do not keep using the pointer after any other function for dynamic arrays are called. Pointer might become invalid
/// </summary>
void* dynamicArrayGetPtr(uint index, DynamicArray dynamicArray);
void dynamicArrayGet(void* data, uint index, DynamicArray dynamicArray);
void dynamicArraySet(void* data, uint index, DynamicArray dynamicArray);

/// <summary>
/// Set all the entries to zero
/// </summary>
void dynamicArrayClear(DynamicArray dynamicArray);


/// <summary>
/// set the allocated size to the occupied size
/// </summary>
void dynamicArrayTrim(DynamicArray dynamicArray);

void dynamicArrayFree(DynamicArray dynamicArray);


