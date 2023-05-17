#pragma once

typedef struct DynamicArray_T* DynamicArray;

void dynamicArrayCreate(uint dataSize, DynamicArray* dynamicArray);
void dynamicArrayDestroy(DynamicArray dynamicArray);

void dynamicArrayAdd(uint64 data, DynamicArray dynamicArray);
void dynamicArrayAddRange(void* data, uint count, DynamicArray dynamicArray);

