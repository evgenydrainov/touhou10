#pragma once

#ifdef __EMSCRIPTEN__
	#include "gles2.h"
#elif defined(_DEBUG)
	#include "gl_debug.h"
#else
	#include "gl_release.h"
#endif
