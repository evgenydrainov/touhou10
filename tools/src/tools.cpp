#define _CRT_SECURE_NO_WARNINGS


#include "tools.h"
#include <SDL.h>
#include <stb/stb_sprintf.h>


// @Copy
void _Assertion_Failed(const char* file, int line, const char* condition) {
	char buf[256];
	stb_snprintf(buf, sizeof(buf), "%s:%d:\n%s", file, line, condition);
	SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Assertion failed", buf, nullptr);
	exit(1);
}


#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb/stb_image_write.h>
#undef STB_IMAGE_WRITE_IMPLEMENTATION

#define STB_RECT_PACK_IMPLEMENTATION
#include <stb/stb_rect_pack.h>
#undef STB_RECT_PACK_IMPLEMENTATION

#define STB_SPRINTF_IMPLEMENTATION
#include <stb/stb_sprintf.h>
#undef STB_SPRINTF_IMPLEMENTATION

