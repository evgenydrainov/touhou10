#define _CRT_SECURE_NO_WARNINGS // for qoi

#pragma warning(push, 0)
#pragma warning(disable : 5045)

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

#define MINICORO_IMPL
#define MCO_DEFAULT_STACK_SIZE 8*1024
#define MCO_MIN_STACK_SIZE 0
#include <minicoro/minicoro.h> // emscripten has to be included before minicoro
#undef MINICORO_IMPL

#define GLAD_GL_IMPLEMENTATION
#define GLAD_GLES2_IMPLEMENTATION
#include <glad/gl.h>
#undef GLAD_GL_IMPLEMENTATION

#define STB_IMAGE_IMPLEMENTATION
#define STBI_ONLY_PNG
#include <stb/stb_image.h>
#undef STB_IMAGE_IMPLEMENTATION

#define STB_SPRINTF_IMPLEMENTATION
#include <stb/stb_sprintf.h>
#undef STB_SPRINTF_IMPLEMENTATION

#define QOI_IMPLEMENTATION
#include <qoi/qoi.h>
#undef QOI_IMPLEMENTATION

#pragma warning(pop)
