#define _CRT_SECURE_NO_WARNINGS

// 
// UNUSED
// 

#include "tools.h"

#ifdef CREATE_FONTS

#ifdef _WIN32
#include <direct.h>
#else
#include <unistd.h>
#endif

struct Font_Desc {
	const char* fnt_filename;
	const char* texture_name;
	const char* font_name;
};

Font_Desc fonts_to_write[] = {
	{"fonts/cirno.fnt", "tex_font_cirno", "fnt_cirno"},
};

int main(int argc, char* argv[]) {

#ifdef _WIN32
	_chdir("../touhou10");
#else
	chdir("../touhou10");
#endif

	FILE* f = fopen("src/font_data___.cpp", "wb");
	defer { fclose(f); };

	fprintf(f, R"(// 
// GENERATED
// 
#include "assets.h"
)");

	for (int i = 0; i < ArrayLength(fonts_to_write); i++) {
		Font_Desc font = fonts_to_write[i];

		fprintf(f, "static Glyph %s_glyphs[] = {\n", font.font_name);



		fprintf(f, "};\n");
	}

	fprintf(f, "Font font_data[NUM_FONTS] = {\n");

	for (int i = 0; i < ArrayLength(fonts_to_write); i++) {
		Font_Desc font = fonts_to_write[i];

		fprintf(f, R"(	/* [%s] = */ {
		/* .glyphs        = */ %s_glyphs,
		/* .num_glyphs    = */ ArrayLength(%s_glyphs),
		/* .texture_index = */ %s,
	},
)", font.font_name, font.font_name, font.font_name, font.texture_name);

	}

	fprintf(f, "};\n");

	return 0;
}

#endif
