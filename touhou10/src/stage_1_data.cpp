#include "scripting.h"
#include <glad/gl.h>

void Stage_1_Script(mco_coro* co) {
	wait(60);

	CreateBoss(BOSS_YOUMU_MIDBOSS);
}


static void draw_world_origin_axis(float delta) {
	static u32 vao;

	const float size = 10;
	const Vertex vertices[] = {
		{{0,    0, 0}, {}, color_red},
		{{size, 0, 0}, {}, color_red},

		{{0, 0,    0}, {}, color_green},
		{{0, size, 0}, {}, color_green},

		{{0, 0, 0},    {}, color_blue},
		{{0, 0, size}, {}, color_blue},
	};

	if (!vao) {
		// @Leak
		vao = create_vertex_array_obj(vertices, ArrayLength(vertices));
	}

	u32 program = r->shader_color_program;

	glUseProgram(program);
	defer { glUseProgram(0); };

	mat4 MVP = w->d3d.get_mvp();

	int u_MVP = glGetUniformLocation(program, "u_MVP");
	glUniformMatrix4fv(u_MVP, 1, GL_FALSE, &MVP[0][0]);

	glBindVertexArray(vao);
	defer { glBindVertexArray(0); };

	glEnable(GL_DEPTH_TEST);
	defer { glDisable(GL_DEPTH_TEST); };

	glDrawArrays(GL_LINES, 0, ArrayLength(vertices));
	r->draw_calls++;
}


static u32 vao;
static int num_vertices;

void Stage_1_Init_Background() {
	w->d3d.cam_pos = {0, 10.0f, 10.0f};
	w->d3d.pitch   = -10;
	w->d3d.yaw     = -90;

	if (!vao) {
		// @Leak
		vao = load_3d_model_from_obj_file("models/pcb_youmu_stairs.obj", &num_vertices);
	}
}

void Stage_1_Draw_Background(float delta) {
	const vec4 fog_color = get_color(0xB08190FFu);

	glClearColor(fog_color.r, fog_color.g, fog_color.b, fog_color.a);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	if (SDL_GetRelativeMouseMode()) {
		draw_world_origin_axis(delta);
	}

	{
		u32 program = r->shader_3d_program;

		glUseProgram(program);
		defer { glUseProgram(0); };

		float y = -wrapf(SDL_GetTicks() / 1000.0f, 8.0f);
		float z =  wrapf(SDL_GetTicks() / 1000.0f, 8.0f);

		mat4 model = glm::translate(mat4{1}, vec3{0, y, z});
		mat4 view  = w->d3d.get_view_mat();
		mat4 proj  = w->d3d.get_proj_mat();

		int u_Model          = glGetUniformLocation(program, "u_Model");
		int u_View           = glGetUniformLocation(program, "u_View");
		int u_Proj           = glGetUniformLocation(program, "u_Proj");
		int u_FogStart       = glGetUniformLocation(program, "u_FogStart");
		int u_FogEnd         = glGetUniformLocation(program, "u_FogEnd");
		int u_FogColor       = glGetUniformLocation(program, "u_FogColor");
		int u_LightDirection = glGetUniformLocation(program, "u_LightDirection");

		glUniformMatrix4fv(u_Model, 1, GL_FALSE, &model[0][0]);
		glUniformMatrix4fv(u_View,  1, GL_FALSE, &view[0][0]);
		glUniformMatrix4fv(u_Proj,  1, GL_FALSE, &proj[0][0]);
		glUniform1f(u_FogStart, 20);
		glUniform1f(u_FogEnd, 40);
		glUniform4fv(u_FogColor, 1, &fog_color[0]);
		glUniform3f(u_LightDirection, 0, -1, 0);

		glBindTexture(GL_TEXTURE_2D, GetTexture(tex_pcb_youmu_stairs)->ID);
		defer { glBindTexture(GL_TEXTURE_2D, 0); };

		glBindVertexArray(vao);
		defer { glBindVertexArray(0); };

		glEnable(GL_DEPTH_TEST);
		defer { glDisable(GL_DEPTH_TEST); };

		glEnable(GL_CULL_FACE);
		defer { glDisable(GL_CULL_FACE); };

		glDrawArrays(GL_TRIANGLES, 0, num_vertices);
		r->draw_calls++;
	}
}
