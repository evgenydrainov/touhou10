#pragma once

#include "common.h"
#include "texture.h"

struct Glyph {
	int u;
	int v;
	int width;
	int height;
	int xoffset;
	int yoffset;
	int xadvance;
};

struct Font {
	Texture atlas;
	bool should_free_atlas;

	array<Glyph> glyphs; // [32..127]
	bool should_free_glyphs;

	int size;
	int line_height;
};

enum HAlign {
	HALIGN_LEFT,
	HALIGN_CENTER,
	HALIGN_RIGHT,
};

enum VAlign {
	VALIGN_TOP,
	VALIGN_MIDDLE,
	VALIGN_BOTTOM,
};

// load files generated by AngelCode's BMFont.
Font load_bmfont_file(const char* fnt_filepath, const char* png_filepath);

// Monospace.
Font load_font_from_texture(const char* filepath,
							int size, int line_height, int char_width,
							int xoffset = 0, int yoffset = 0);

void free_font(Font* f);

// Returns the position of the next-to-be-drawn character.
// No Unicode, only Ascii.
vec2 draw_text(const Font& font, string text, vec2 text_pos,
			   HAlign halign = HALIGN_LEFT, VAlign valign = VALIGN_TOP, vec4 color = color_white);

vec2 draw_text_shadow(const Font& font, string text, vec2 text_pos,
					  HAlign halign = HALIGN_LEFT, VAlign valign = VALIGN_TOP, vec4 color = color_white);

// The height of an empty string is font.size
vec2 measure_text(const Font& font, string text, bool only_one_line = false);
