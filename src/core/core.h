#pragma once
#define AV_LOG_CATEGORY "avixel"
#include <avixel/avixel.h>
#include "util/util.h"
#include "logging/logging.h"
#include "renderer/renderer.h"
#include "positioner/positioner.h"
#include "components/component.h"
#include "math/math.h"

typedef struct RenderInstance_T* RenderInstance;
typedef struct RenderDevice_T* RenderDevice;
typedef struct DisplaySurface_T* DisplaySurface;
typedef struct Window_T* Window;
typedef struct Pipeline_T* Pipeline;

typedef struct AvInstance_T {
	DisplaySurface displaySurface;
	RenderInstance renderInstance;
	Window window;
	RenderDevice renderDevice;
}AvInstance_T;

typedef struct AvWindow_T {

} AvWindow_T;



 