
void* avAllocate_(uint size, uint count, AV_LOCATION_ARGS, AV_CATEGORY_ARGS, const char* msg);
#define avAllocate(size,count,message) avAllocate_(size,count,AV_LOCATION_PARAMS, AV_LOG_CATEGORY, message)

void avFree_(void* data, AV_LOCATION_ARGS);
#define avFree(data) avFree_(data, AV_LOCATION_PARAMS)