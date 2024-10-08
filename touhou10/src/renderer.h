#pragma once

#include "common.h"

constexpr size_t BATCH_MAX_QUADS    = 10'000;
constexpr size_t VERTICES_PER_QUAD  = 4;
constexpr size_t INDICES_PER_QUAD   = 6;
constexpr size_t BATCH_MAX_VERTICES = (BATCH_MAX_QUADS * VERTICES_PER_QUAD);
constexpr size_t BATCH_MAX_INDICES  = (BATCH_MAX_QUADS * INDICES_PER_QUAD);

struct Renderer;
struct Texture;
struct Sprite;
struct Font;

extern Renderer* r;

struct Vertex {
	vec3 pos;
	vec3 normal;
	vec4 color;
	vec2 uv;
};

constexpr vec4 color_white  = { 1.00f, 1.00f, 1.00f, 1.00f };
constexpr vec4 color_black  = { 0.00f, 0.00f, 0.00f, 1.00f };
constexpr vec4 color_red    = { 1.00f, 0.00f, 0.00f, 1.00f };
constexpr vec4 color_green  = { 0.00f, 1.00f, 0.00f, 1.00f };
constexpr vec4 color_blue   = { 0.00f, 0.00f, 1.00f, 1.00f };
constexpr vec4 color_yellow = { 1.00f, 1.00f, 0.00f, 1.00f };

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

u32 create_vertex_array_obj(const Vertex* vertices, size_t num_vertices,
							const u32* indices = nullptr, size_t num_indices = 0,
							u32* out_vbo = nullptr, u32* out_ebo = nullptr);

void set_vertex_attribs();

struct Renderer {

	enum Mode {
		MODE_NONE,
		MODE_QUADS,
		MODE_TRIANGLES,
	};

	Mode mode;

	u32 shader_texture_program;
	u32 shader_color_program;
	u32 shader_misty_lake_program;
	u32 shader_sharp_bilinear_program;
	u32 shader_3d_program;

	u32 current_texture_shader;
	u32 current_color_shader;

	dynamic_array_cap<Vertex> batch_vertices;
	u32 batch_vao;
	u32 batch_vbo;
	u32 batch_ebo;
	u32 batch_texture;

	mat4 model;
	mat4 view;
	mat4 proj;

	int draw_calls;
	size_t max_batch;

	void init();
	void destroy();

	void load_shaders();

	void draw_texture(Texture* t, Rect src = {},
					  vec2 pos = {}, vec2 scale = {1.0f, 1.0f},
					  vec2 origin = {}, float angle = 0.0f, vec4 color = color_white, glm::bvec2 flip = {});

	void draw_rectangle(Rect rect, vec4 color);

	// @Temp This is temporary. Draws a 16x16 rect
	void draw_rectangle_ext(vec2 pos, vec2 scale = {1.0f, 1.0f},
							vec2 origin = {}, float angle = 0.0f, vec4 color = color_white);

	void draw_sprite(Sprite* s, int frame_index,
					 vec2 pos, vec2 scale = {1.0f, 1.0f},
					 float angle = 0.0f, vec4 color = color_white, glm::bvec2 flip = {});

	void draw_triangle(vec2 p1, vec2 p2, vec2 p3, vec4 color = color_white);

	void draw_circle(vec2 pos, float radius, vec4 color = color_white, int precision = 6);

	vec2 draw_text(Sprite* font, string text, float x, float y,
				   HAlign halign = HALIGN_LEFT, VAlign valign = VALIGN_TOP, vec4 color = color_white);

	vec2 measure_text(Sprite* font, string text, bool only_one_line = false);

	vec2 draw_text(Font* font, string text, float x, float y,
				   HAlign halign = HALIGN_LEFT, VAlign valign = VALIGN_TOP, vec4 color = color_white);

	vec2 measure_text(Font* font, string text, bool only_one_line = false);

	vec2 draw_text_shadow(Font* font, string text, float x, float y,
						  HAlign halign = HALIGN_LEFT, VAlign valign = VALIGN_TOP, vec4 color = color_white);

	void break_batch();

};
