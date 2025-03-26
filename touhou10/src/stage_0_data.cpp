#include "renderer.h"
#include "util.h"
#include "scripting.h"
#include "window_creation.h"

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
			int r = random_range(&world.rng, 0, 3);
			float dir = random_rangef(&world.rng, 0.0f, 360.0f);
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

		float x = random_rangef(&world.rng, 0.0f, (float)PLAY_AREA_W);
		float y = 0;

		CreateEnemy(x, y, 2, 270 + random_rangef(&world.rng, -30.0f, 30.0f), 0, spr_enemy_0, 10, 1, Script, OnDeath, OnUpdate);
	};


	if (game.skip_to_boss) {
		goto l_boss;
	}

	if (game.skip_to_midboss) {
		goto l_midboss;
	}


	wait(120);

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

				CreateEnemy(x, y, 2, point_direction(x, y, world.player.x, world.player.y), 0, spr_fairy_0, 10, 2);

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
		while (world.find_boss(dai_chan)) {
			wait(1);
		}
	}

	wait(60);

l_boss:

	{
		instance_id baka = CreateBoss(BOSS_CIRNO)->id;
		// while (world.find_boss(baka)) {
		// 	wait(1);
		// }
	}

#if 0
	CreateBoss(MIDBOSS_YOUMU);
#endif
}

static float speed;
static float fog_start;

void Stage_0_Init_Background() {
	world.cam3d.pos   = {0, 1, 0};
	world.cam3d.pitch = -45;
	world.cam3d.yaw   = -90;
	speed = 0;
	fog_start = -5;
}

void Stage_0_Draw_Background(float delta) {
	delta /= 60.0f;

	break_batch();

	Approach(&speed, 0.1f, 0.02f * delta);
	Approach(&fog_start, 1.0f, 2.0f * delta);

	world.cam3d.pos.z -= speed * delta;
	world.cam3d.pos.z = wrapf(world.cam3d.pos.z, 1.0f);

	static u32 vao;
	static u32 vbo;
	static u32 ebo;

	static bool got_uniforms;
	static int u_MVP;
	static int u_Texture;
	static int u_Offset;
	static int u_FogStart;
	static int u_FogEnd;
	static int u_FogColor;
	static int u_Color;

	Vertex vertices[] = {
		{{-10, 0, -10}, {}, color_white, {-10, -10}},
		{{ 10, 0, -10}, {}, color_white, { 10, -10}},
		{{ 10, 0,  10}, {}, color_white, { 10,  10}},
		{{-10, 0,  10}, {}, color_white, {-10,  10}},
	};

	u32 indices[] = {
		0, 1, 2,
		2, 3, 0,
	};

	if (!vao) {
		// @Leak
		vao = create_vertex_array_obj(vertices, ArrayLength(vertices), indices, ArrayLength(indices), &vbo, &ebo);
	}

	u32 program = get_shader(shd_stage0_bg).id;

	if (!got_uniforms) {
		u_MVP      = glGetUniformLocation(program, "u_MVP");
		u_Texture  = glGetUniformLocation(program, "u_Texture");
		u_Offset   = glGetUniformLocation(program, "u_Offset");
		u_FogStart = glGetUniformLocation(program, "u_FogStart");
		u_FogEnd   = glGetUniformLocation(program, "u_FogEnd");
		u_FogColor = glGetUniformLocation(program, "u_FogColor");
		u_Color    = glGetUniformLocation(program, "u_Color");

		got_uniforms = true;
	}

	const vec4 fog_color = {1, 1, 1, 1};

	glClearColor(fog_color.r, fog_color.g, fog_color.b, fog_color.a);
	glClear(GL_COLOR_BUFFER_BIT);

	{
		glUseProgram(program);

		mat4 model = mat4{1.0f};
		mat4 view  = world.cam3d_get_view_mat();
		mat4 proj  = world.cam3d_get_proj_mat();
		mat4 mvp = (proj * view) * model;

		float time = (float)get_time();

		vec2 offset = {0, time * 0.09f};
		vec4 col = color_white;

		glUniformMatrix4fv(u_MVP, 1, GL_FALSE, &mvp[0][0]);
		glUniform1i(u_Texture, 0);
		glUniform2fv(u_Offset, 1, &offset[0]);
		glUniform1f(u_FogStart, fog_start);
		glUniform1f(u_FogEnd, 6);
		glUniform4fv(u_FogColor, 1, &fog_color[0]);
		glUniform4fv(u_Color, 1, &col[0]);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, get_texture(tex_gfw_misty_lake).id);

		glBindVertexArray(vao);

		glDrawElements(GL_TRIANGLES, ArrayLength(indices), GL_UNSIGNED_INT, 0);
		renderer.curr_draw_calls++;

		{
			model = get_translation({0, 0.01f, 0});
			mvp = (proj * view) * model;
			offset = {time * 0.05f, 0};
			col = {1, 1, 1, 0.5f};

			glUniformMatrix4fv(u_MVP, 1, GL_FALSE, &mvp[0][0]);
			glUniform2fv(u_Offset, 1, &offset[0]);
			glUniform4fv(u_Color, 1, &col[0]);

			glDrawElements(GL_TRIANGLES, ArrayLength(indices), GL_UNSIGNED_INT, 0);
			renderer.curr_draw_calls++;
		}

		{
			model = get_translation({0, 0.02f, 0});
			mvp = (proj * view) * model;
			offset = {-time * 0.01f, -time * 0.01f};
			col = {1, 1, 1, 0.35f};

			glUniformMatrix4fv(u_MVP, 1, GL_FALSE, &mvp[0][0]);
			glUniform2fv(u_Offset, 1, &offset[0]);
			glUniform4fv(u_Color, 1, &col[0]);

			glBindTexture(GL_TEXTURE_2D, get_texture(tex_gfw_misty_lake2).id);

			glDrawElements(GL_TRIANGLES, ArrayLength(indices), GL_UNSIGNED_INT, 0);
			renderer.curr_draw_calls++;
		}

		{
			model = get_translation({0, 0.03f, 0});
			mvp = (proj * view) * model;
			offset = {time * 0.013f, -time * 0.009f};
			col = {1, 1, 1, 0.75f};

			glUniformMatrix4fv(u_MVP, 1, GL_FALSE, &mvp[0][0]);
			glUniform2fv(u_Offset, 1, &offset[0]);
			glUniform4fv(u_Color, 1, &col[0]);

			glBindTexture(GL_TEXTURE_2D, get_texture(tex_gfw_misty_lake3).id);

			glDrawElements(GL_TRIANGLES, ArrayLength(indices), GL_UNSIGNED_INT, 0);
			renderer.curr_draw_calls++;
		}

		{
			model = get_translation({0, 0.04f, 0});
			mvp = (proj * view) * model;
			offset = {time * 0.006f, -time * 0.002f};
			col = {1, 1, 1, 0.75f};

			glUniformMatrix4fv(u_MVP, 1, GL_FALSE, &mvp[0][0]);
			glUniform2fv(u_Offset, 1, &offset[0]);
			glUniform4fv(u_Color, 1, &col[0]);

			glBindTexture(GL_TEXTURE_2D, get_texture(tex_gfw_misty_lake4).id);

			glDrawElements(GL_TRIANGLES, ArrayLength(indices), GL_UNSIGNED_INT, 0);
			renderer.curr_draw_calls++;
		}

		glBindVertexArray(0);
		glBindTexture(GL_TEXTURE_2D, 0);

		glUseProgram(0);
	}
}
