#include "scripting.h"
#include "game.h"
#include "renderer.h"
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

		CreateEnemy(x, y, 2, 270 + random_rangef(&world.rng, -30.0f, 30.0f), 0, enemy_pcb_spinner, 10, 1, Script, OnDeath, OnUpdate);
	};


	if (game.skip_to_boss) {
		goto l_boss;
	}

	if (game.skip_to_midboss) {
		goto l_midboss;
	}


	wait(120);

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

				CreateEnemy(x, y, 2, point_direction(x, y, world.player.x, world.player.y), 0, enemy_fairy_blue, 10, 2);

				if (i >= 12) {
					spawn_spinner();
				}

				wait(20);
			}

			wait(60);
		}

		wait(180);
	}

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
		while (world.find_boss(baka)) {
			wait(1);
		}
	}
}

static float fog_density;

void Stage_0_Init_Background() {
	world.cam3d.pos   = {0, 0.90f, 0};
	world.cam3d.pitch = -45;
	world.cam3d.yaw   = -90;
	fog_density = 1.0f;
}

void Stage_0_Update_Background(float delta) {
	delta /= 60.0f;

	world.cam3d.pos.z -= 0.05f * delta;
	world.cam3d.pos.z = wrapf(world.cam3d.pos.z, 1.0f);

	Approach(&fog_density, 0.4f, 0.25f*delta);
}

void Stage_0_Draw_Background(float delta) {
	delta /= 60.0f;

	static bool got_uniforms;
	static int u_FogStart;
	static int u_FogEnd;
	static int u_FogColor;
	static int u_FogDensity;

	u32 program = get_shader(shd_basic_3d).id;

	if (!got_uniforms) {
		u_FogStart   = glGetUniformLocation(program, "u_FogStart");
		u_FogEnd     = glGetUniformLocation(program, "u_FogEnd");
		u_FogColor   = glGetUniformLocation(program, "u_FogColor");
		u_FogDensity = glGetUniformLocation(program, "u_FogDensity");

		got_uniforms = true;
	}

	const vec4 fog_color = {1, 1, 1, 1};
	float t = (float)get_time();

	mat4 view =
		world.cam3d_get_view_mat()
		* get_rotation(to_radians(sinf(t * 0.5f) * 2), vec3{0, 0, 1});

	render_clear_color(fog_color);
	set_proj_mat(world.cam3d_get_proj_mat());
	set_view_mat(view);

	set_shader(program);

	glUniform1f(u_FogStart, 0);
	glUniform1f(u_FogEnd, 5);
	glUniform1f(u_FogDensity, fog_density);
	glUniform4fv(u_FogColor, 1, &fog_color[0]);

	float pos_y = 0;

	{
		vec4 color = color_white;
		Vertex vertices[] = {
			{{-10, pos_y, -10}, {0, 1, 0}, color, {-10 + t*0.025f, -10 - t*0.05f}},
			{{ 10, pos_y, -10}, {0, 1, 0}, color, { 10 + t*0.025f, -10 - t*0.05f}},
			{{ 10, pos_y,  10}, {0, 1, 0}, color, { 10 + t*0.025f,  10 - t*0.05f}},
			{{-10, pos_y,  10}, {0, 1, 0}, color, {-10 + t*0.025f,  10 - t*0.05f}},
		};

		draw_quad(get_texture(tex_gfw_misty_lake), vertices);
		pos_y += 0.01f;
	}

	{
		vec4 color = {1, 1, 1, 0.5f};
		Vertex vertices[] = {
			{{-10, pos_y, -10}, {0, 1, 0}, color, {-10 + t*0.05f, -10 - t*0.025f}},
			{{ 10, pos_y, -10}, {0, 1, 0}, color, { 10 + t*0.05f, -10 - t*0.025f}},
			{{ 10, pos_y,  10}, {0, 1, 0}, color, { 10 + t*0.05f,  10 - t*0.025f}},
			{{-10, pos_y,  10}, {0, 1, 0}, color, {-10 + t*0.05f,  10 - t*0.025f}},
		};

		draw_quad(get_texture(tex_gfw_misty_lake), vertices);
		pos_y += 0.01f;
	}

	{
		vec4 color = {1, 1, 1, 0.25f + sinf(t*0.5f)*0.15f};
		Vertex vertices[] = {
			{{-10, pos_y, -10}, {0, 1, 0}, color, {-10 - t*0.02f, -10 - t*0.02f}},
			{{ 10, pos_y, -10}, {0, 1, 0}, color, { 10 - t*0.02f, -10 - t*0.02f}},
			{{ 10, pos_y,  10}, {0, 1, 0}, color, { 10 - t*0.02f,  10 - t*0.02f}},
			{{-10, pos_y,  10}, {0, 1, 0}, color, {-10 - t*0.02f,  10 - t*0.02f}},
		};

		draw_quad(get_texture(tex_gfw_misty_lake2), vertices);
		pos_y += 0.01f;
	}

	{
		vec4 color = {1, 1, 1, 0.80f};
		Vertex vertices[] = {
			{{-10, pos_y, -10}, {0, 1, 0}, color, {-10 + t*0.025f, -10 - t*0.05f}},
			{{ 10, pos_y, -10}, {0, 1, 0}, color, { 10 + t*0.025f, -10 - t*0.05f}},
			{{ 10, pos_y,  10}, {0, 1, 0}, color, { 10 + t*0.025f,  10 - t*0.05f}},
			{{-10, pos_y,  10}, {0, 1, 0}, color, {-10 + t*0.025f,  10 - t*0.05f}},
		};

		draw_quad(get_texture(tex_gfw_misty_lake3), vertices);
		pos_y += 0.01f;
	}

	{
		vec4 color = {1, 1, 1, 0.80f};
		Vertex vertices[] = {
			{{-10, pos_y, -10}, {0, 1, 0}, color, {-10 + t*0.031f, -10 - t*0.04f}},
			{{ 10, pos_y, -10}, {0, 1, 0}, color, { 10 + t*0.031f, -10 - t*0.04f}},
			{{ 10, pos_y,  10}, {0, 1, 0}, color, { 10 + t*0.031f,  10 - t*0.04f}},
			{{-10, pos_y,  10}, {0, 1, 0}, color, {-10 + t*0.031f,  10 - t*0.04f}},
		};

		draw_quad(get_texture(tex_gfw_misty_lake4), vertices);
		pos_y += 0.01f;
	}

	// cleanup
	reset_shader();
	set_proj_mat(get_ortho(0, PLAY_AREA_W, PLAY_AREA_H, 0));
	set_view_mat(get_identity());
}
