#pragma once

#include "common.h"
#include "array.h"
#include <glm/glm.hpp>

constexpr size_t BATCH_MAX_QUADS = 10'000;
constexpr size_t VERTICES_PER_QUAD = 4;
constexpr size_t INDICES_PER_QUAD = 6;
constexpr size_t BATCH_MAX_VERTICES = (BATCH_MAX_QUADS * VERTICES_PER_QUAD);
constexpr size_t BATCH_MAX_INDICES = (BATCH_MAX_QUADS * INDICES_PER_QUAD);

struct Renderer;
struct Texture;
struct Sprite;

extern Renderer* r;

struct Vertex {
	glm::vec3 pos;
	glm::vec4 color;
	glm::vec2 uv;
};

constexpr glm::vec4 color_white = {1.0f, 1.0f, 1.0f, 1.0f};
constexpr glm::vec4 color_black = {0.0f, 0.0f, 0.0f, 1.0f};
constexpr glm::vec4 color_red   = {1.0f, 0.0f, 0.0f, 1.0f};
constexpr glm::vec4 color_green = {0.0f, 1.0f, 0.0f, 1.0f};
constexpr glm::vec4 color_blue  = {0.0f, 0.0f, 1.0f, 1.0f};

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

struct Renderer {

	enum Mode {
		MODE_NONE,
		MODE_QUADS,
		MODE_TRIANGLES,
	};

	Mode mode;

	u32 shader_texture_program;
	u32 shader_color_program;
	u32 shader_stage_0_bg_program;

	u32 quad_vao;
	u32 quad_vbo;
	u32 quad_ebo;

	u32 triangle_vao;
	u32 triangle_vbo;

	Arena_Backed_Array<Vertex> batch_vertices;
	u32 batch_vao;
	u32 batch_vbo;
	u32 batch_ebo;
	u32 batch_texture;

	glm::mat4 model;
	glm::mat4 view;
	glm::mat4 proj;

	int draw_calls;
	size_t max_batch;

	void init();
	void destroy();

	void draw_texture(Texture* t, Rect src = {},
					  glm::vec2 pos = {}, glm::vec2 scale = {1.0f, 1.0f},
					  glm::vec2 origin = {}, float angle = 0.0f, glm::vec4 color = color_white, glm::bvec2 flip = {});

	void draw_rectangle(Rect rect, glm::vec4 color);

	// @Temp This is temporary. Draws a 16x16 rect
	void draw_rectangle_ext(glm::vec2 pos, glm::vec2 scale = {1.0f, 1.0f},
							glm::vec2 origin = {}, float angle = 0.0f, glm::vec4 color = color_white);

	void draw_sprite(Sprite* s, int frame_index,
					 glm::vec2 pos, glm::vec2 scale = {1.0f, 1.0f},
					 float angle = 0.0f, glm::vec4 color = color_white, glm::bvec2 flip = {});

	void draw_triangle(glm::vec2 p1, glm::vec2 p2, glm::vec2 p3, glm::vec4 color = color_white);

	void draw_circle(glm::vec2 pos, float radius, glm::vec4 color = color_white, int precision = 6);

	glm::vec2 draw_text(Sprite* font, const char* text, float x, float y,
						HAlign halign = HALIGN_LEFT, VAlign valign = VALIGN_TOP);

	glm::vec2 measure_text(Sprite* font, const char* text, bool only_one_line = false);

	void break_batch();

};
