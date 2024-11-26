#pragma once

#include "common.h"

inline u32 compile_shader(GLenum type, const char* source) {
	u32 shader = glCreateShader(type);

	const char* sources[] = {source};
	glShaderSource(shader, ArrayLength(sources), sources, NULL);

	glCompileShader(shader);

	int success;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
	if (!success) {
		char buf[512];
		glGetShaderInfoLog(shader, sizeof(buf), NULL, buf);
		log_error("Shader Compile Error: %s", buf);
	}

	return shader;
}

inline u32 link_program(u32 vertex_shader, u32 fragment_shader) {
	u32 program = glCreateProgram();

	glAttachShader(program, vertex_shader);
	glAttachShader(program, fragment_shader);

	glLinkProgram(program);

	int success;
	glGetProgramiv(program, GL_LINK_STATUS, &success);
	if (!success) {
		char buf[512];
		glGetProgramInfoLog(program, sizeof(buf), NULL, buf);
		log_error("Shader Link Error: %s", buf);
	}

	return program;
}

inline u32 create_vertex_array_obj(const Vertex* vertices, size_t num_vertices,
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
