#ifndef __GRAPHICS_UTILITIES_INCLUDED__
#define __GRAPHICS_UTILITIES_INCLUDED__


#ifdef __cplusplus
extern "C" {
#endif

#ifdef __LOCAL_LIB__
#include "AvGraphicsCore.h"
#else
#include <GraphicsUtilities/AvGraphicsCore.h>
#endif

#ifndef PLATFORM_EMBEDDED

#ifdef __LOCAL_LIB__
#include "AvUiParser.h"
#else
#include <GraphicsUtilities/AvUiParser.h>
#endif

#endif


#ifdef __cplusplus
}
#endif

#endif//__GRAPHICS_UTILITIES_INCLUDED__