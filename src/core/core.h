#pragma once
#define __LOCAL_LIB__
#include <GraphicsUtilities.h>
#include "util/util.h"
#include "logging/logging.h"
#include "renderer/renderer.h"
#include "positioner/positioner.h"
#include "components/component.h"

typedef struct RenderInstance_T* RenderInstance;
typedef struct RenderDevice_T* RenderDevice;
typedef struct DisplaySurface_T* DisplaySurface;
typedef struct Window_T* Window;

typedef struct AvInstance_T {
	DisplaySurface displaySurface;
	RenderInstance renderInstance;
	Window window;
	RenderDevice renderDevice;
}AvInstance_T;



 