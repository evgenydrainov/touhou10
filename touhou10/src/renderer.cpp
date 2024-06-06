#include "renderer.h"

#include "game.h"
#include "cpml.h"

#include <glad/gl.h>
#include <glm/gtc/matrix_transform.hpp>

Renderer* r;

void Renderer::init() {

	// 
	// Load shaders.
	// 
	{

		auto compile_shader = [](GLenum type, const char* source) {
			u32 shader = glCreateShader(type);

			const char* sources[] = {source};
			glShaderSource(shader, ArrayLength(sources), sources, NULL);

			glCompileShader(shader);

			return shader;
		};

		auto link_program = [](u32 vertex_shader, u32 fragment_shader) {
			u32 program = glCreateProgram();

			glAttachShader(program, vertex_shader);
			glAttachShader(program, fragment_shader);

			glLinkProgram(program);

			return program;
		};

		u32 shader_texture_vertex = compile_shader(GL_VERTEX_SHADER, shader_texture_vertex_text);
		Defer { glDeleteShader(shader_texture_vertex); };

		u32 shader_texture_fragment = compile_shader(GL_FRAGMENT_SHADER, shader_texture_fragment_text);
		Defer { glDeleteShader(shader_texture_fragment); };

		u32 shader_color_fragment = compile_shader(GL_FRAGMENT_SHADER, shader_color_fragment_text);
		Defer { glDeleteShader(shader_color_fragment); };

		u32 shader_stage_0_bg_vertex = compile_shader(GL_VERTEX_SHADER, shader_stage_0_bg_vertex_text);
		Defer { glDeleteShader(shader_stage_0_bg_vertex); };

		u32 shader_stage_0_bg_fragment = compile_shader(GL_FRAGMENT_SHADER, shader_stage_0_bg_fragment_text);
		Defer { glDeleteShader(shader_stage_0_bg_fragment); };

		shader_texture_program = link_program(shader_texture_vertex, shader_texture_fragment);
		shader_color_program = link_program(shader_texture_vertex, shader_color_fragment);
		shader_stage_0_bg_program = link_program(shader_stage_0_bg_vertex, shader_stage_0_bg_fragment);
	}



	// 
	// Initialize.
	// 
	{
		glGenVertexArrays(1, &quad_vao);
		glGenBuffers(1, &quad_vbo);
		glGenBuffers(1, &quad_ebo);

		// 1. bind Vertex Array Object
		glBindVertexArray(quad_vao);

		// 2. copy our vertices array in a vertex buffer for OpenGL to use
		glBindBuffer(GL_ARRAY_BUFFER, quad_vbo);
		glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * 4, nullptr, GL_DYNAMIC_DRAW);

		u32 indices[] = {
			0, 1, 2,
			2, 3, 0,
		};

		// 3. copy our index array in a element buffer for OpenGL to use
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, quad_ebo);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

		// 4. then set the vertex attributes pointers
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, pos)); // position
		glEnableVertexAttribArray(0);

		glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, color)); // color
		glEnableVertexAttribArray(1);

		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, uv)); // texcoord
		glEnableVertexAttribArray(2);

		glBindVertexArray(0);
	}

	{
		glGenVertexArrays(1, &triangle_vao);
		glGenBuffers(1, &triangle_vbo);

		// 1. bind Vertex Array Object
		glBindVertexArray(triangle_vao);

		// 2. copy our vertices array in a vertex buffer for OpenGL to use
		glBindBuffer(GL_ARRAY_BUFFER, triangle_vbo);
		glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * 3, nullptr, GL_DYNAMIC_DRAW);

		// 4. then set the vertex attributes pointers
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, pos)); // position
		glEnableVertexAttribArray(0);

		glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, color)); // color
		glEnableVertexAttribArray(1);

		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, uv)); // texcoord
		glEnableVertexAttribArray(2);

		glBindVertexArray(0);
	}

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
			const size_t memory_for_indices = BATCH_MAX_INDICES * sizeof(u32);
			const size_t memory_for_vertices = BATCH_MAX_VERTICES * sizeof(Vertex);
			static_assert(memory_for_indices <= memory_for_vertices, "there's not gonna be enough memory in the arena");
		}

		size_t arena_pos = ArenaGetPos(&g->arena);

		u32* indices = PushArray(&g->arena, u32, BATCH_MAX_INDICES);

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

		ArenaSetPosBack(&g->arena, arena_pos);

		// 4. then set the vertex attributes pointers
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, pos)); // position
		glEnableVertexAttribArray(0);

		glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, color)); // color
		glEnableVertexAttribArray(1);

		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, uv)); // texcoord
		glEnableVertexAttribArray(2);

		glBindVertexArray(0);

		batch_vertices = array_from_arena<Vertex>(&g->arena, BATCH_MAX_VERTICES);
	}

	model = {1.0f};
	view = {1.0f};
	proj = {1.0f};

}

void Renderer::destroy() {
	glDeleteBuffers(1, &batch_ebo);
	glDeleteBuffers(1, &batch_vbo);
	glDeleteVertexArrays(1, &batch_vao);

	glDeleteBuffers(1, &triangle_vbo);
	glDeleteVertexArrays(1, &triangle_vao);

	glDeleteBuffers(1, &quad_ebo);
	glDeleteBuffers(1, &quad_vbo);
	glDeleteVertexArrays(1, &quad_vao);

	glDeleteProgram(shader_stage_0_bg_program);
	glDeleteProgram(shader_color_program);
	glDeleteProgram(shader_texture_program);
}

void Renderer::draw_texture(Texture* t, Rect src,
							glm::vec2 pos, glm::vec2 scale,
							glm::vec2 origin, float angle, glm::vec4 color, glm::bvec2 flip) {

	Assert(t);

	if (src.w == 0) {
		src.w = t->width;
	}
	if (src.h == 0) {
		src.h = t->height;
	}

	if (t->ID != batch_texture) {
		break_batch();
		batch_texture = t->ID;
	}

	if (mode != MODE_QUADS) {
		break_batch();
		mode = MODE_QUADS;
	}

	{
		float x1 = -origin.x;
		float y1 = -origin.y;
		float x2 = (float)src.w - origin.x;
		float y2 = (float)src.h - origin.y;

		float u1 = (float)src.x / (float)t->width;
		float v1 = (float)src.y / (float)t->height;
		float u2 = (float)(src.x + src.w) / (float)t->width;
		float v2 = (float)(src.y + src.h) / (float)t->height;

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
			{{x1, y1, 0.0f}, color, {u1, v1}},
			{{x2, y1, 0.0f}, color, {u2, v1}},
			{{x2, y2, 0.0f}, color, {u2, v2}},
			{{x1, y2, 0.0f}, color, {u1, v2}},
		};

		// glBindBuffer(GL_ARRAY_BUFFER, quad_vbo);
		// glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
		// glBindBuffer(GL_ARRAY_BUFFER, 0);

		glm::mat4 model = glm::translate(glm::mat4{1.0f}, {pos.x, pos.y, 0.0f});
		model = glm::scale(model, {scale.x, scale.y, 1.0f});
		model = glm::rotate(model, glm::radians(-angle), {0.0f, 0.0f, 1.0f});

		vertices[0].pos = model * glm::vec4{vertices[0].pos, 1.0f};
		vertices[1].pos = model * glm::vec4{vertices[1].pos, 1.0f};
		vertices[2].pos = model * glm::vec4{vertices[2].pos, 1.0f};
		vertices[3].pos = model * glm::vec4{vertices[3].pos, 1.0f};

		batch_vertices.add(vertices[0]);
		batch_vertices.add(vertices[1]);
		batch_vertices.add(vertices[2]);
		batch_vertices.add(vertices[3]);
	}

#if 0
	// 
	// Draw.
	// 
	{
		glUseProgram(shader_texture_program);

		model = glm::translate(glm::mat4{1.0f}, {pos.x, pos.y, 0.0f});
		model = glm::scale(model, {scale.x, scale.y, 1.0f});
		model = glm::rotate(model, glm::radians(-angle), {0.0f, 0.0f, 1.0f});

		glm::mat4 MVP = (proj * view) * model;

		int u_MVP = glGetUniformLocation(shader_texture_program, "u_MVP");
		glUniformMatrix4fv(u_MVP, 1, GL_FALSE, &MVP[0][0]);

		glBindTexture(GL_TEXTURE_2D, t->ID);
		glBindVertexArray(quad_vao);

		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
		draw_calls++;

		glBindVertexArray(0);
		glBindTexture(GL_TEXTURE_2D, 0);

		model = {1.0f};

		glUseProgram(0);
	}
#endif

}

void Renderer::draw_rectangle(Rect rect, glm::vec4 color) {
#if 0
	{
		float x1 = (float)rect.x;
		float y1 = (float)rect.y;
		float x2 = (float)(rect.x + rect.w);
		float y2 = (float)(rect.y + rect.h);

		Vertex vertices[] = {
			{{x1, y1, 0.0f}, color, {}},
			{{x2, y1, 0.0f}, color, {}},
			{{x2, y2, 0.0f}, color, {}},
			{{x1, y2, 0.0f}, color, {}},
		};

		glBindBuffer(GL_ARRAY_BUFFER, quad_vbo);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}

	{
		glUseProgram(shader_color_program);

		glm::mat4 MVP = (proj * view) * model;

		int u_MVP = glGetUniformLocation(shader_color_program, "u_MVP");
		glUniformMatrix4fv(u_MVP, 1, GL_FALSE, &MVP[0][0]);

		glBindVertexArray(quad_vao);

		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
		draw_calls++;

		glBindVertexArray(0);
		glUseProgram(0);
	}
#else
	draw_sprite(GetSprite(spr_white), 0, {rect.x, rect.y}, {rect.w / 16.0f, rect.h / 16.0f}, 0.0f, color);
#endif
}

void Renderer::draw_sprite(Sprite* s, int frame_index,
						   glm::vec2 pos, glm::vec2 scale,
						   float angle, glm::vec4 color, glm::bvec2 flip) {
	Texture* t = GetTexture(s->texture_index);

	SpriteFrame* frame = &s->frames[frame_index];

	draw_texture(t, {frame->u, frame->v, frame->w, frame->h},
				 pos, scale, {(float)s->xorigin, (float)s->yorigin}, angle, color, flip);
}

void Renderer::draw_triangle(glm::vec2 p1, glm::vec2 p2, glm::vec2 p3, glm::vec4 color) {

	if (mode != MODE_TRIANGLES) {
		break_batch();
		mode = MODE_TRIANGLES;
	}

	{
		Vertex vertices[] = {
			{{p1, 0.0f}, color, {}},
			{{p2, 0.0f}, color, {}},
			{{p3, 0.0f}, color, {}},
		};

		// glBindBuffer(GL_ARRAY_BUFFER, triangle_vbo);
		// glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
		// glBindBuffer(GL_ARRAY_BUFFER, 0);

		batch_vertices.add(vertices[0]);
		batch_vertices.add(vertices[1]);
		batch_vertices.add(vertices[2]);
	}

#if 0
	{
		glUseProgram(shader_color_program);

		glm::mat4 MVP = (proj * view) * model;

		int u_MVP = glGetUniformLocation(shader_color_program, "u_MVP");
		glUniformMatrix4fv(u_MVP, 1, GL_FALSE, &MVP[0][0]);

		glBindVertexArray(triangle_vao);

		glDrawArrays(GL_TRIANGLES, 0, 3);
		draw_calls++;

		glBindVertexArray(0);
		glUseProgram(0);
	}
#endif
}

void Renderer::draw_circle(glm::vec2 pos, float radius, glm::vec4 color, int precision) {
	for (int i = 0; i < precision; i++) {
		float angle1 = (float)i       / (float)precision * 2.0f * glm::pi<float>();
		float angle2 = (float)(i + 1) / (float)precision * 2.0f * glm::pi<float>();
		glm::vec2 p1 = pos + glm::vec2{cosf(angle1), -sinf(angle1)} * radius;
		glm::vec2 p2 = pos + glm::vec2{cosf(angle2), -sinf(angle2)} * radius;
		draw_triangle(p1, p2, pos, color);
	}
}

glm::vec2 Renderer::draw_text(Sprite* font, const char* text, float x, float y,
							  HAlign halign, VAlign valign) {

	switch (valign) {
		case VALIGN_MIDDLE:
			y -= floorf(measure_text(font, text).y / 2.0f);
			break;
		case VALIGN_BOTTOM:
			y -= measure_text(font, text).y;
			break;
	}

	float ch_x = x;
	float ch_y = y;

	switch (halign) {
		case HALIGN_CENTER:
			ch_x -= floorf(measure_text(font, text, true).x / 2.0f);
			break;
		case HALIGN_RIGHT:
			ch_x -= measure_text(font, text, true).x;
			break;
	}

	int ch;
	for (const char* ptr = text; ch = *ptr; ptr++) {
		if (33 <= ch && ch <= 127) {
			draw_sprite(font, ch - 32, {ch_x, ch_y});
		}

		ch_x += (float)font->width;

		if (ch == '\n') {
			ch_x = x;
			ch_y += (float)font->height;

			switch (halign) {
				case HALIGN_CENTER:
					ch_x -= floorf(measure_text(font, ptr + 1, true).x / 2.0f);
					break;
				case HALIGN_RIGHT:
					ch_x -= measure_text(font, ptr + 1, true).x;
					break;
			}
		}
	}

	return {ch_x, ch_y};
}

glm::vec2 Renderer::measure_text(Sprite* font, const char* text, bool only_one_line) {

	float w = 0;
	float h = (float)font->height;

	float ch_x = 0;
	float ch_y = 0;

	int ch;
	for (const char* ptr = text; ch = *ptr; ptr++) {
		if (33 <= ch && ch <= 127) {
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
			glUseProgram(shader_texture_program);
			Defer { glUseProgram(0); };

			glm::mat4 MVP = (proj * view) * model;

			int u_MVP = glGetUniformLocation(shader_texture_program, "u_MVP");
			glUniformMatrix4fv(u_MVP, 1, GL_FALSE, &MVP[0][0]);

			glBindTexture(GL_TEXTURE_2D, batch_texture);
			Defer { glBindTexture(GL_TEXTURE_2D, 0); };

			glBindVertexArray(batch_vao);
			Defer { glBindVertexArray(0); };

			Assert(batch_vertices.count % 4 == 0);

			glDrawElements(GL_TRIANGLES, (GLsizei)batch_vertices.count / 4 * 6, GL_UNSIGNED_INT, 0);
			draw_calls++;
			max_batch = max(max_batch, batch_vertices.count);
			break;
		}

		case MODE_TRIANGLES: {
			glUseProgram(shader_color_program);
			Defer { glUseProgram(0); };

			glm::mat4 MVP = (proj * view) * model;

			int u_MVP = glGetUniformLocation(shader_color_program, "u_MVP");
			glUniformMatrix4fv(u_MVP, 1, GL_FALSE, &MVP[0][0]);

			glBindVertexArray(batch_vao);
			Defer { glBindVertexArray(0); };

			Assert(batch_vertices.count % 3 == 0);

			glDrawArrays(GL_TRIANGLES, 0, (GLsizei)batch_vertices.count);
			draw_calls++;
			max_batch = max(max_batch, batch_vertices.count);
			break;
		}
	}

	batch_vertices.clear();
	batch_texture = 0;
	mode = MODE_NONE;
}
