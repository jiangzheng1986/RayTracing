
#pragma once

#pragma warning(disable: 4996)

#include <math.h>

#ifndef M_PI_F
#define M_PI_F				3.14159265358979323846f
#endif

#define EPSILON_VALUE_1		0.001f

#ifndef MAX_
#define MAX_(a,b)            (((a) > (b)) ? (a) : (b))
#endif

#ifndef MIN_
#define MIN_(a,b)            (((a) < (b)) ? (a) : (b))
#endif

#define ABS_(a)				((a) < 0 ? (-a) : (a))

#include <time.h>

#include <windows.h>

#include <Gdiplus.h>

#include <stdlib.h>

#include <vector>

#include "RayTracing.h"

#include "Soft3DEngine/CSoft3DEngine.h"

#include "Soft3DEngine.h"