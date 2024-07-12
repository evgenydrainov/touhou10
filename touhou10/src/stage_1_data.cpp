#include "scripting.h"
#include <glad/gl.h>
#include <glm/gtc/matrix_transform.hpp>

void Stage_1_Script(mco_coro* co) {
	Wait(60);

	CreateBoss(BOSS_YOUMU_MIDBOSS);
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
	glClearColor(1, 1, 1, 1);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	{
		u32 program = r->shader_texture_program;

		glUseProgram(program);
		Defer { glUseProgram(0); };

		glm::mat4 MVP = w->d3d.get_mvp();
		MVP = glm::translate(MVP, {0, -wrapf(SDL_GetTicks() / 1000.0f, 8.0f), wrapf(SDL_GetTicks() / 1000.0f, 8.0f)});

		int u_MVP = glGetUniformLocation(program, "u_MVP");
		glUniformMatrix4fv(u_MVP, 1, GL_FALSE, &MVP[0][0]);

		glBindTexture(GL_TEXTURE_2D, GetTexture(tex_pcb_youmu_stairs)->ID);
		Defer { glBindTexture(GL_TEXTURE_2D, 0); };

		glBindVertexArray(vao);
		Defer { glBindVertexArray(0); };

		glEnable(GL_DEPTH_TEST);
		Defer { glDisable(GL_DEPTH_TEST); };

		glEnable(GL_CULL_FACE);
		Defer { glDisable(GL_CULL_FACE); };

		glDrawArrays(GL_TRIANGLES, 0, num_vertices);
		r->draw_calls++;
	}
}
