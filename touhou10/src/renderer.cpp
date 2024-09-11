#include "renderer.h"

#include "game.h"

#include <glad/gl.h>

Renderer* r;

void Renderer::init() {


	// 
	// Initialize.
	// 
	{
		glGenVertexArrays(1, &batch_vao);
		glGenBuffers(1, &batch_vbo);
		glGenBuffers(1, &batch_ebo);

		// 1. bind Vertex Array Object
		glBindVertexArray(batch_vao);

		// 2. copy our vertices array in a vertex buffer for OpenGL to use
		glBindBuffer(GL_ARRAY_BUFFER, batch_vbo);
		glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * BATCH_MAX_VERTICES, nullptr, GL_DYNAMIC_DRAW);

		// 
		// Use arena temporarily to generate indices
		// 
		{
			const size_t memory_for_indices  = BATCH_MAX_INDICES  * sizeof(u32);
			const size_t memory_for_vertices = BATCH_MAX_VERTICES * sizeof(Vertex);

			static_assert(memory_for_indices <= memory_for_vertices, "there's not gonna be enough memory in the arena");
		}

		size_t arena_pos = g->arena.count;

		u32* indices = (u32*) arena_push(&g->arena, BATCH_MAX_INDICES * sizeof(u32));

		u32 offset = 0;
		for (size_t i = 0; i < BATCH_MAX_INDICES; i += 6) {
			indices[i + 0] = offset + 0;
			indices[i + 1] = offset + 1;
			indices[i + 2] = offset + 2;

			indices[i + 3] = offset + 2;
			indices[i + 4] = offset + 3;
			indices[i + 5] = offset + 0;

			offset += 4;
		}

		// 3. copy our index array in a element buffer for OpenGL to use
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, batch_ebo);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(u32) * BATCH_MAX_INDICES, indices, GL_STATIC_DRAW);

		g->arena.count = arena_pos;

		// 4. then set the vertex attributes pointers
		set_vertex_attribs();

		glBindVertexArray(0);

		batch_vertices = dynamic_array_cap_from_arena<Vertex>(&g->arena, BATCH_MAX_VERTICES);
	}

	model = {1.0f};
	view  = {1.0f};
	proj  = {1.0f};

	// Initialize after loading shaders
	Assert(shader_texture_program != 0);
	Assert(shader_color_program   != 0);

	current_texture_shader = shader_texture_program;
	current_color_shader   = shader_color_program;

}

void Renderer::destroy() {
	glDeleteBuffers(1, &batch_ebo);
	glDeleteBuffers(1, &batch_vbo);
	glDeleteVertexArrays(1, &batch_vao);
}

void Renderer::load_shaders() {

	// 
	// Load shaders.
	// 
	{

		string shader_include = copy_string(get_file_string("shaders/include.glsl"));
		defer { free(shader_include.data); };

		auto compile_shader = [&](GLenum type, string filename) {
			string source = get_file_string(filename);

			string define_string = {};
			if (type == GL_VERTEX_SHADER) {
				define_string =
					"#version 330 core\n"
					"#define VERTEX_SHADER\n";
			} else if (type == GL_FRAGMENT_SHADER) {
				define_string =
					"#version 330 core\n"
					"#define FRAGMENT_SHADER\n";
			}

			u32 shader = glCreateShader(type);

			const char* sources[] = {
				define_string.data,
				shader_include.data,
				source.data,
			};
			int lengths[] = {
				(int)define_string.count,
				(int)shader_include.count,
				(int)source.count,
			};

			Assert(ArrayLength(sources) == ArrayLength(lengths));

			glShaderSource(shader, ArrayLength(sources), sources, lengths);

			glCompileShader(shader);

			int success;
			glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
			if (!success) {
				char buf[512];
				glGetShaderInfoLog(shader, sizeof(buf), NULL, buf);
				log_error("SHADER COMPILATION ERROR: %s", buf);
			} else {
				string str = {};
				if (type == GL_VERTEX_SHADER) {
					str = "fragment";
				} else if (type == GL_FRAGMENT_SHADER) {
					str = "vertex";
				}

				log_info("Loaded " Str_Fmt " shader " Str_Fmt, Str_Arg(str), Str_Arg(filename));
			}

			return shader;
		};

		auto link_program = [&](u32 vertex_shader, u32 fragment_shader) {
			u32 program = glCreateProgram();

			glAttachShader(program, vertex_shader);
			glAttachShader(program, fragment_shader);

			glLinkProgram(program);

			int success;
			glGetProgramiv(program, GL_LINK_STATUS, &success);
			if (!success) {
				char buf[512];
				glGetProgramInfoLog(program, sizeof(buf), NULL, buf);
				log_error("SHADER LINKING ERROR: %s", buf);
			}

			return program;
		};


		// 
		// @Todo: Don't load the shader files twice.
		// 

		u32 shader_texture_vertex = compile_shader(GL_VERTEX_SHADER, "shaders/texture.glsl");
		defer { glDeleteShader(shader_texture_vertex); };

		u32 shader_texture_fragment = compile_shader(GL_FRAGMENT_SHADER, "shaders/texture.glsl");
		defer { glDeleteShader(shader_texture_fragment); };

		u32 shader_color_fragment = compile_shader(GL_FRAGMENT_SHADER, "shaders/color.glsl");
		defer { glDeleteShader(shader_color_fragment); };

		u32 shader_misty_lake_vertex = compile_shader(GL_VERTEX_SHADER, "shaders/misty_lake.glsl");
		defer { glDeleteShader(shader_misty_lake_vertex); };

		u32 shader_misty_lake_fragment = compile_shader(GL_FRAGMENT_SHADER, "shaders/misty_lake.glsl");
		defer { glDeleteShader(shader_misty_lake_fragment); };

		u32 shader_sharp_bilinear_fragment = compile_shader(GL_FRAGMENT_SHADER, "shaders/sharp_bilinear.glsl");
		defer { glDeleteShader(shader_sharp_bilinear_fragment); };

		u32 shader_3d_fragment = compile_shader(GL_FRAGMENT_SHADER, "shaders/3d.glsl");
		defer { glDeleteShader(shader_3d_fragment); };

		u32 shader_3d_vertex = compile_shader(GL_VERTEX_SHADER, "shaders/3d.glsl");
		defer { glDeleteShader(shader_3d_vertex); };

		// @Leak
		shader_texture_program        = link_program(shader_texture_vertex,    shader_texture_fragment);
		shader_color_program          = link_program(shader_texture_vertex,    shader_color_fragment);
		shader_misty_lake_program     = link_program(shader_misty_lake_vertex, shader_misty_lake_fragment);
		shader_sharp_bilinear_program = link_program(shader_texture_vertex,    shader_sharp_bilinear_fragment);
		shader_3d_program             = link_program(shader_3d_vertex,         shader_3d_fragment);
	}

}

void Renderer::draw_texture(Texture* t, Rect src,
							vec2 pos, vec2 scale,
							vec2 origin, float angle, vec4 color, glm::bvec2 flip) {

	Assert(t);

#if DRAW_AT_FLOORED_POSITION
	pos = glm::floor(pos);
#endif

	if (src.w == 0 && src.h == 0) {
		src.w = t->width;
		src.h = t->height;
	}

	if (t->ID != batch_texture || mode != MODE_QUADS) {
		break_batch();

		batch_texture = t->ID;
		mode = MODE_QUADS;
	}

	{
		float x1 = -origin.x;
		float y1 = -origin.y;
		float x2 = src.w - origin.x;
		float y2 = src.h - origin.y;

		float u1 =  src.x          / (float)t->width;
		float v1 =  src.y          / (float)t->height;
		float u2 = (src.x + src.w) / (float)t->width;
		float v2 = (src.y + src.h) / (float)t->height;

		if (flip.x) {
			float temp = u1;
			u1 = u2;
			u2 = temp;
		}

		if (flip.y) {
			float temp = v1;
			v1 = v2;
			v2 = temp;
		}

		Vertex vertices[] = {
			{{x1, y1, 0.0f}, {}, color, {u1, v1}},
			{{x2, y1, 0.0f}, {}, color, {u2, v1}},
			{{x2, y2, 0.0f}, {}, color, {u2, v2}},
			{{x1, y2, 0.0f}, {}, color, {u1, v2}},
		};

		// Has to be in this order (learned it the hard way)
		mat4 model = glm::translate(mat4{1.0f}, {pos.x, pos.y, 0.0f});
		model = glm::rotate(model, glm::radians(-angle), {0.0f, 0.0f, 1.0f});
		model = glm::scale(model, {scale.x, scale.y, 1.0f});

		vertices[0].pos = model * vec4{vertices[0].pos, 1.0f};
		vertices[1].pos = model * vec4{vertices[1].pos, 1.0f};
		vertices[2].pos = model * vec4{vertices[2].pos, 1.0f};
		vertices[3].pos = model * vec4{vertices[3].pos, 1.0f};

		array_add(&batch_vertices, vertices[0]);
		array_add(&batch_vertices, vertices[1]);
		array_add(&batch_vertices, vertices[2]);
		array_add(&batch_vertices, vertices[3]);
	}

}

void Renderer::draw_rectangle(Rect rect, vec4 color) {
	draw_sprite(GetSprite(spr_white), 0, {rect.x, rect.y}, {rect.w / 16.0f, rect.h / 16.0f}, 0.0f, color);
}

void Renderer::draw_rectangle_ext(vec2 pos, vec2 scale,
								  vec2 origin, float angle, vec4 color) {
	Sprite* s = GetSprite(spr_white);
	Texture* t = GetTexture(s->texture_index);
	SpriteFrame f = s->frames[0];
	draw_texture(t, {f.u, f.v, f.w, f.h}, pos, scale, origin, angle, color);
}

void Renderer::draw_sprite(Sprite* s, int frame_index,
						   vec2 pos, vec2 scale,
						   float angle, vec4 color, glm::bvec2 flip) {
	Texture* t = GetTexture(s->texture_index);

	Assert(frame_index >= 0);
	Assert(frame_index < s->frame_count);

	SpriteFrame* frame = &s->frames[frame_index];

	draw_texture(t, {frame->u, frame->v, frame->w, frame->h},
				 pos, scale, {(float)s->xorigin, (float)s->yorigin}, angle, color, flip);
}

void Renderer::draw_triangle(vec2 p1, vec2 p2, vec2 p3, vec4 color) {

	if (mode != MODE_TRIANGLES) {
		break_batch();
		mode = MODE_TRIANGLES;
	}

	{
		Vertex vertices[] = {
			{{p1.x, p1.y, 0.0f}, {}, color, {}},
			{{p2.x, p2.y, 0.0f}, {}, color, {}},
			{{p3.x, p3.y, 0.0f}, {}, color, {}},
		};

		array_add(&batch_vertices, vertices[0]);
		array_add(&batch_vertices, vertices[1]);
		array_add(&batch_vertices, vertices[2]);
	}
}

void Renderer::draw_circle(vec2 pos, float radius, vec4 color, int precision) {
	for (int i = 0; i < precision; i++) {
		float angle1 = (float)i       / (float)precision * 2.0f * glm::pi<float>();
		float angle2 = (float)(i + 1) / (float)precision * 2.0f * glm::pi<float>();
		vec2 p1 = pos + vec2{cosf(angle1), -sinf(angle1)} * radius;
		vec2 p2 = pos + vec2{cosf(angle2), -sinf(angle2)} * radius;
		draw_triangle(p1, p2, pos, color);
	}
}

vec2 Renderer::draw_text(Sprite* font, string text, float x, float y,
						 HAlign halign, VAlign valign, vec4 color) {

	switch (valign) {
		case VALIGN_MIDDLE:
			y -= measure_text(font, text).y / 2.0f;
			break;
		case VALIGN_BOTTOM:
			y -= measure_text(font, text).y;
			break;
	}

	float ch_x = x;
	float ch_y = y;

	switch (halign) {
		case HALIGN_CENTER:
			ch_x -= measure_text(font, text, true).x / 2.0f;
			break;
		case HALIGN_RIGHT:
			ch_x -= measure_text(font, text, true).x;
			break;
	}

	for (size_t i = 0; i < text.count; i++) {
		u8 ch = (u8) text[i];

		// If char isn't valid, set to '?'
		if (!((ch >= 32 && ch <= 127) || ch == '\n')) {
			ch = '?';
		}

		// If char isn't whitespace, draw it
		if (!(ch == ' ' || ch == '\n')) {
			draw_sprite(font, ch - 32, {floorf(ch_x), floorf(ch_y)}, {1, 1}, 0, color);
		}

		ch_x += (float)font->width;

		if (ch == '\n') {
			ch_x = x;
			ch_y += (float)font->height;

			switch (halign) {
				case HALIGN_CENTER:
					ch_x -= measure_text(font, {text.data + i + 1, text.count - i - 1}, true).x / 2.0f;
					break;
				case HALIGN_RIGHT:
					ch_x -= measure_text(font, {text.data + i + 1, text.count - i - 1}, true).x;
					break;
			}
		}
	}

	return {ch_x, ch_y};
}

vec2 Renderer::measure_text(Sprite* font, string text, bool only_one_line) {
	float w = 0;
	float h = (float)font->height;

	float ch_x = 0;
	float ch_y = 0;

	for (size_t i = 0; i < text.count; i++) {
		u8 ch = (u8) text[i];

		if (!((ch >= 32 && ch <= 127) || ch == '\n')) {
			ch = '?';
		}

		if (!(ch == ' ' || ch == '\n')) {
			w = max(w, ch_x + (float)font->width);
			h = max(h, ch_y + (float)font->height);
		}

		ch_x += (float)font->width;

		if (ch == '\n') {
			if (only_one_line) {
				return {w, h};
			}

			ch_x = 0;
			ch_y += (float)font->height;

			h = max(h, ch_y + (float)font->height);
		}
	}

	return {w, h};
}

vec2 Renderer::draw_text(Font* font, string text, float x, float y,
						 HAlign halign, VAlign valign, vec4 color) {

	switch (valign) {
		case VALIGN_MIDDLE:
			y -= measure_text(font, text).y / 2.0f;
			break;
		case VALIGN_BOTTOM:
			y -= measure_text(font, text).y;
			break;
	}

	float ch_x = x;
	float ch_y = y;

	switch (halign) {
		case HALIGN_CENTER:
			ch_x -= measure_text(font, text, true).x / 2.0f;
			break;
		case HALIGN_RIGHT:
			ch_x -= measure_text(font, text, true).x;
			break;
	}

	for (size_t i = 0; i < text.count; i++) {
		u8 ch = (u8) text[i];

		// If char isn't valid, set it to '?'
		if (!((ch >= 32 && ch <= 127) || ch == '\n')) {
			ch = '?';
		}

		if (ch == '\n') {
			ch_x = x;
			ch_y += font->line_height;

			switch (halign) {
				case HALIGN_CENTER:
					ch_x -= measure_text(font, {text.data + i + 1, text.count - i - 1}, true).x / 2.0f;
					break;
				case HALIGN_RIGHT:
					ch_x -= measure_text(font, {text.data + i + 1, text.count - i - 1}, true).x;
					break;
			}
		} else {
			Assert(font->num_glyphs == 95);
			Glyph glyph = font->glyphs[ch - 32];

			// If char isn't whitespace, draw it
			if (ch != ' ') {
				Texture* t = GetTexture(font->texture_index);
				Rect src = {glyph.u, glyph.v, glyph.width, glyph.height};

				vec2 pos;
				pos.x = ch_x + glyph.xoffset;
				pos.y = ch_y + glyph.yoffset;

				pos = glm::floor(pos);

				draw_texture(t, src, pos, {1, 1}, {}, 0, color);
			}

			ch_x += glyph.xadvance;
		}
	}

	return {ch_x, ch_y};
}

vec2 Renderer::measure_text(Font* font, string text, bool only_one_line) {
	float w = 0;
	float h = font->size;

	float ch_x = 0;
	float ch_y = 0;

	for (size_t i = 0; i < text.count; i++) {
		u8 ch = (u8) text[i];

		// If char isn't valid, set it to '?'
		if (!((ch >= 32 && ch <= 127) || ch == '\n')) {
			ch = '?';
		}

		if (ch == '\n') {
			if (only_one_line) return {w, h};

			ch_x = 0;
			ch_y += font->line_height;
		} else {
			Assert(font->num_glyphs == 95);
			Glyph glyph = font->glyphs[ch - 32];

			// If char isn't whitespace, draw it
			if (ch != ' ') {
				vec2 pos;
				pos.x = ch_x + glyph.xoffset;
				pos.y = ch_y + glyph.yoffset;

				pos = glm::floor(pos);

				w = max(w, pos.x + glyph.width);
				h = max(h, pos.y + font->size);
			}

			ch_x += glyph.xadvance;
		}
	}

	return {w, h};
}

vec2 Renderer::draw_text_shadow(Font* font, string text, float x, float y,
								HAlign halign, VAlign valign, vec4 color) {
	draw_text(font, text, x + 1, y + 1, halign, valign, color_black);
	vec2 result = draw_text(font, text, x, y, halign, valign, color);
	return result;
}

void Renderer::break_batch() {
	if (batch_vertices.count == 0) {
		return;
	}

	Assert(mode != MODE_NONE);

	{
		glBindBuffer(GL_ARRAY_BUFFER, batch_vbo);
		glBufferSubData(GL_ARRAY_BUFFER, 0, batch_vertices.count * sizeof(Vertex), batch_vertices.data);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}

	switch (mode) {
		case MODE_QUADS: {
			u32 program = current_texture_shader;

			glUseProgram(program);
			defer { glUseProgram(0); };

			mat4 MVP = (proj * view) * model;

			int u_MVP = glGetUniformLocation(program, "u_MVP");
			glUniformMatrix4fv(u_MVP, 1, GL_FALSE, &MVP[0][0]);

			glBindTexture(GL_TEXTURE_2D, batch_texture);
			defer { glBindTexture(GL_TEXTURE_2D, 0); };

			glBindVertexArray(batch_vao);
			defer { glBindVertexArray(0); };

			Assert(batch_vertices.count % 4 == 0);

			glDrawElements(GL_TRIANGLES, (GLsizei)batch_vertices.count / 4 * 6, GL_UNSIGNED_INT, 0);
			draw_calls++;
			max_batch = max(max_batch, batch_vertices.count);
			break;
		}

		case MODE_TRIANGLES: {
			u32 program = current_color_shader;

			glUseProgram(program);
			defer { glUseProgram(0); };

			mat4 MVP = (proj * view) * model;

			int u_MVP = glGetUniformLocation(program, "u_MVP");
			glUniformMatrix4fv(u_MVP, 1, GL_FALSE, &MVP[0][0]);

			glBindVertexArray(batch_vao);
			defer { glBindVertexArray(0); };

			Assert(batch_vertices.count % 3 == 0);

			glDrawArrays(GL_TRIANGLES, 0, (GLsizei)batch_vertices.count);
			draw_calls++;
			max_batch = max(max_batch, batch_vertices.count);
			break;
		}
	}

	batch_vertices.count = 0;
	batch_texture = 0;
	mode = MODE_NONE;
}

u32 create_vertex_array_obj(const Vertex* vertices, size_t num_vertices,
							const u32* indices, size_t num_indices,
							u32* out_vbo, u32* out_ebo) {
	u32 vao = 0;
	u32 vbo = 0;
	u32 ebo = 0;

	glGenVertexArrays(1, &vao);
	glGenBuffers(1, &vbo);
	if (indices) glGenBuffers(1, &ebo);

	glBindVertexArray(vao);
	defer { glBindVertexArray(0); };

	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, num_vertices * sizeof(vertices[0]), vertices, GL_STATIC_DRAW);

	if (indices) {
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, num_indices * sizeof(indices[0]), indices, GL_STATIC_DRAW);
	}

	set_vertex_attribs();

	if (out_vbo) *out_vbo = vbo;
	if (out_ebo) *out_ebo = ebo;
	return vao;
}

void set_vertex_attribs() {
	// Position
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, pos));
	glEnableVertexAttribArray(0);

	// Normal
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
	glEnableVertexAttribArray(1);

	// Color
	glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, color));
	glEnableVertexAttribArray(2);

	// Texcoord
	glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, uv));
	glEnableVertexAttribArray(3);
}
