#pragma once

#include "common.h"
#include "renderer.h"

inline u32 compile_shader(GLenum type, string source, const char* debug_name = nullptr) {
	u32 shader = glCreateShader(type);

#if defined(__ANDROID__)
	string version_string = "#version 320 es\n";
#elif defined(__EMSCRIPTEN__)
	string version_string = "#version 300 es\n";
#else
	string version_string = "#version 330 core\n";
#endif

	string precision_string = "#ifdef GL_ES\n"
		"precision highp float;\n"
		"#endif\n";

	const char* sources[] = {version_string.data, precision_string.data, source.data};
	int lengths[] = {version_string.count, precision_string.count, source.count};
	glShaderSource(shader, ArrayLength(sources), sources, lengths);

	glCompileShader(shader);

	int success;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
	if (!success) {
		char buf[512];
		glGetShaderInfoLog(shader, sizeof(buf), NULL, buf);

		if (debug_name) log_error("While compiling %s...", debug_name);
		log_error("Shader Compile Error:\n%s", buf);
	}

	return shader;
}

inline u32 link_program(u32 vertex_shader, u32 fragment_shader, const char* debug_name = nullptr) {
	u32 program = glCreateProgram();

	glAttachShader(program, vertex_shader);
	glAttachShader(program, fragment_shader);

	glLinkProgram(program);

	int success;
	glGetProgramiv(program, GL_LINK_STATUS, &success);
	if (!success) {
		char buf[512];
		glGetProgramInfoLog(program, sizeof(buf), NULL, buf);

		if (debug_name) log_error("While linking %s...", debug_name);
		log_error("Shader Link Error:\n%s", buf);
	}

	return program;
}

inline u32 create_vertex_array_obj(const Vertex* vertices, size_t num_vertices,
								   const u32* indices = nullptr, size_t num_indices = 0,
								   u32* out_vbo = nullptr, u32* out_ebo = nullptr) {
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
