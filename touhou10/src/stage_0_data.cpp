#include "scripting.h"
#include <glad/gl.h>

void Stage_0_Script(mco_coro* co) {

	auto spawn_spinner = [&]() {
		auto Script = [](mco_coro* co) {
			wait(60);
			self->acc = -0.01f;
			while (self->spd > 0) {
				wait(1);
			}
			self->dir = -self->dir;
			self->acc = 0.01f;
		};

		auto OnDeath = [](Object* o) {
			u32 r = w->random.next() % 3;
			float dir = w->random.rangef(0.0f, 360.0f);
			if (r == 0) {
				ShootO(o, 2, dir,       0, spr_bullet_rice, 2);
				ShootO(o, 2, dir +  90, 0, spr_bullet_rice, 2);
				ShootO(o, 2, dir + 180, 0, spr_bullet_rice, 2);
				ShootO(o, 2, dir + 270, 0, spr_bullet_rice, 2);
			} else if (r == 1) {
				ShootO(o, 3, dir,       0, spr_bullet_kunai, 10);
				ShootO(o, 3, dir +  90, 0, spr_bullet_kunai, 10);
				ShootO(o, 3, dir + 180, 0, spr_bullet_kunai, 10);
				ShootO(o, 3, dir + 270, 0, spr_bullet_kunai, 10);
			}
		};

		auto OnUpdate = [](Object* o, float delta) {
			((Enemy*)o)->angle += 10 * delta;
		};

		float x = w->random.rangef(0.0f, (float)PLAY_AREA_W);
		float y = 0;

		CreateEnemy(x, y, 2, 270 + w->random.rangef(-30.0f, 30.0f), 0, spr_enemy_0, 10, 1, Script, OnDeath, OnUpdate);
	};


	if (g->skip_to_boss) {
		goto l_boss;
	}

	if (g->skip_to_midboss) {
		goto l_midboss;
	}


	wait(60);

#if 1
	{
		Repeat (50) {
			spawn_spinner();
			wait(5);
		}

		wait(180);

		for (int j = 1; j <= 4; j++) {
			for (int i = 1; i <= 18; i++) {
				float x = i * (PLAY_AREA_W / 19.0f);
				if (j % 2 == 0) {
					x = PLAY_AREA_W - x;
				}
				float y = 0;

				CreateEnemy(x, y, 2, point_direction(x, y, w->player.x, w->player.y), 0, spr_fairy_0, 10, 2);

				if (i >= 12) {
					spawn_spinner();
				}

				wait(20);
			}

			wait(60);
		}

		wait(180);
	}
#endif

l_midboss:

	{
		instance_id dai_chan = CreateBoss(BOSS_DAIYOUSEI_MIDBOSS)->id;
		while (w->find_boss(dai_chan)) {
			wait(1);
		}
	}

	wait(60);

l_boss:

	{
		instance_id baka = CreateBoss(BOSS_CIRNO)->id;
		// while (w->find_boss(baka)) {
		// 	wait(1);
		// }
	}

#if 0
	CreateBoss(MIDBOSS_YOUMU);
#endif
}


static u32 vao;
static u32 vbo;
static u32 ebo;

static const u32 indices[] = {
	0, 1, 2,
	2, 3, 0,
};

void Stage_0_Init_Background() {
	w->d3d.cam_pos = {0, 10, 0};
	w->d3d.pitch   = -45;
	w->d3d.yaw     = -90;

	if (!vao) {
		Assert(!vbo);
		Assert(!ebo);

		float x1 = -100;
		float z1 = -100;
		float x2 = 100;
		float z2 = 100;

		float u1 = 0;
		float v1 = 0;
		float u2 = 1;
		float v2 = 1;

		Vertex vertices[] = {
			{{x1, 0, z1}, {}, color_white, {u1, v1}},
			{{x2, 0, z1}, {}, color_white, {u2, v1}},
			{{x2, 0, z2}, {}, color_white, {u2, v2}},
			{{x1, 0, z2}, {}, color_white, {u1, v2}},
		};

		// @Leak
		vao = create_vertex_array_obj(vertices, ArrayLength(vertices), indices, ArrayLength(indices), &vbo, &ebo);
	}
}

void Stage_0_Draw_Background(float delta) {
	const vec4 fog_color = {1, 1, 1, 1};

	glClearColor(fog_color.r, fog_color.g, fog_color.b, fog_color.a);
	glClear(GL_COLOR_BUFFER_BIT);

	{
		u32 program = r->shader_stage_0_bg_program;

		glUseProgram(program);

		mat4 model = {1};
		mat4 view  = w->d3d.get_view_mat();
		mat4 proj  = w->d3d.get_proj_mat();

		int u_Model    = glGetUniformLocation(program, "u_Model");
		int u_View     = glGetUniformLocation(program, "u_View");
		int u_Proj     = glGetUniformLocation(program, "u_Proj");
		int u_Time     = glGetUniformLocation(program, "u_Time");
		int u_FogStart = glGetUniformLocation(program, "u_FogStart");
		int u_FogEnd   = glGetUniformLocation(program, "u_FogEnd");
		int u_FogColor = glGetUniformLocation(program, "u_FogColor");

		glUniformMatrix4fv(u_Model, 1, GL_FALSE, &model[0][0]);
		glUniformMatrix4fv(u_View,  1, GL_FALSE, &view[0][0]);
		glUniformMatrix4fv(u_Proj,  1, GL_FALSE, &proj[0][0]);
		glUniform1f(u_Time, SDL_GetTicks() / 1000.0f);
		glUniform1f(u_FogStart, 20);
		glUniform1f(u_FogEnd, 60);
		glUniform4fv(u_FogColor, 1, &fog_color[0]);

		glBindTexture(GL_TEXTURE_2D, GetTexture(tex_eosd_misty_lake)->ID);
		glBindVertexArray(vao);

		glDrawElements(GL_TRIANGLES, ArrayLength(indices), GL_UNSIGNED_INT, 0);
		r->draw_calls++;

		glBindVertexArray(0);
		glBindTexture(GL_TEXTURE_2D, 0);

		glUseProgram(0);
	}
}
