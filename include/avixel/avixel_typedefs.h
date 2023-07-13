
typedef unsigned int uint;
typedef unsigned char byte;

#ifndef __cplusplus
#ifndef DISABLE_BOOL_TYPEDEF
typedef unsigned char bool;
#define true 1
#define false 0
#endif
#ifndef DISABLE_NULLPTR_TYPEDEF
#define nullptr ((void*)0)
#endif
#endif

#ifndef DISABLE_TYPEDEFS
typedef unsigned long long uint64;
typedef unsigned int uint32;
typedef unsigned short uint16;
typedef unsigned char uint8;

typedef unsigned long int64;
typedef unsigned int int32;
typedef unsigned short int16;
typedef unsigned char int8;
#endif

typedef struct Color {
	union {
		struct {
			byte r;
			byte g;
			byte b;
			byte a;
		};
		uint rgba;
	};
} Color;