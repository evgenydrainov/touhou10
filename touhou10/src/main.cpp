#define _CRT_SECURE_NO_WARNINGS // for qoi

#include "game.h"

#ifdef __EMSCRIPTEN__
#include <emscripten.h>

static void emscripten_main_loop() {
	g->tick();
}

#endif

int main(int argc, char* argv[]) {
	Game game = {};

	// Yeah I know. Single letter global variables.
	g = &game;

	g->init();

#ifdef __EMSCRIPTEN__
	emscripten_set_main_loop(emscripten_main_loop, 60, 1);
#else
	g->run();
#endif

	g->destroy();

	return 0;
}

#pragma warning(push, 0)
#pragma warning(disable : 5045)

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
