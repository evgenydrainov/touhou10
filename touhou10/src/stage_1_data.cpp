#include "scripting.h"
#include <glad/gl.h>

void Stage_1_Script(mco_coro* co) {
	Wait(60);

	CreateBoss(BOSS_YOUMU_MIDBOSS);
}


static void draw_world_origin_axis(float delta) {
	static u32 vao;

	const float size = 10;
	const Vertex vertices[] = {
		{{0,    0, 0}, color_red},
		{{size, 0, 0}, color_red},

		{{0, 0,    0}, color_green},
		{{0, size, 0}, color_green},

		{{0, 0, 0},    color_blue},
		{{0, 0, size}, color_blue},
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
	w->d3d.cam_pos = {0, 15.6419, 7.4495};
	w->d3d.pitch   = -(90 - 65.9595f);
	w->d3d.yaw     = -90;

	if (!vao) {
		// @Leak
		vao = load_3d_model_from_obj_file("models/pcb_youmu_stairs.obj", &num_vertices);
	}
}

void Stage_1_Draw_Background(float delta) {
	const glm::vec4 fog_color = get_color(0xB08190FFu);

	glClearColor(fog_color.r, fog_color.g, fog_color.b, fog_color.a);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	if (SDL_GetRelativeMouseMode()) {
		draw_world_origin_axis(delta);
	}

	{
		u32 program = r->shader_3d_program;

		glUseProgram(program);
		defer { glUseProgram(0); };

		mat4 MVP = w->d3d.get_mvp();
		MVP = glm::translate(MVP, {0, -wrapf(SDL_GetTicks() / 1000.0f, 8.0f), wrapf(SDL_GetTicks() / 1000.0f, 8.0f)});

		int u_MVP      = glGetUniformLocation(program, "u_MVP");
		int u_FogStart = glGetUniformLocation(program, "u_FogStart");
		int u_FogEnd   = glGetUniformLocation(program, "u_FogEnd");
		int u_FogColor = glGetUniformLocation(program, "u_FogColor");

		glUniformMatrix4fv(u_MVP, 1, GL_FALSE, &MVP[0][0]);
		glUniform1f(u_FogStart,   15);
		glUniform1f(u_FogEnd,     30);
		glUniform4fv(u_FogColor,  1, &fog_color[0]);

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
