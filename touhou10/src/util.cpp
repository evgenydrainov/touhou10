#include "util.h"

#include "package.h"

Shader load_shader_from_file(const char* vert_fname, const char* frag_fname) {
	string vertex_source = get_file_str(vert_fname);
	u32 vertex_shader = compile_shader(GL_VERTEX_SHADER, vertex_source, vert_fname);
	defer { glDeleteShader(vertex_shader); };

	string fragment_source = get_file_str(frag_fname);
	u32 fragment_shader = compile_shader(GL_FRAGMENT_SHADER, fragment_source, frag_fname);
	defer { glDeleteShader(fragment_shader); };

	Shader shader = {};
	shader.id = link_program(vertex_shader, fragment_shader, vert_fname);

	return shader;
}
