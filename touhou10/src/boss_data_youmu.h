#pragma once

// 
// "M" for Midboss.
// 
static void M_Youmu_Nonspell_0(mco_coro* co) {
	auto shoot = [&](int N, float x, float y, float dir, u32 sprite_index, int frame_index) {
		ShootRadial(N, 180 / (float)N, [&]() {
			return ShootExt(x, y, 1.15f, dir, 0, sprite_index, frame_index);
		});
	};

	auto slash = [&](float start_x, float start_y, float target_x, float target_y, u32 sprite_index, int frame_index) {
		int N = 12;
		for (int i = 0; i < N; i++) {
			float x = start_x + 25.0f * (i - N / 2.0f);
			float y = start_y;

			for (int i = 0; i < 4; i++) {
				ShootExt(x, y, lerp(2.0f, 4.5f, i / 3.0f), point_direction(x, y, target_x, target_y), 0, sprite_index, frame_index);
			}

			wait(2);
		}
	};

	while (true) {
		Repeat (2) {
			float dir = DirToPlayer(self);
			float startlen = 200;
			float startx = self->x + lengthdir_x(startlen, dir);
			float starty = self->y + lengthdir_y(startlen, dir);
			float endlen = 330;
			float endx = startx - lengthdir_x(endlen, dir);
			float endy = starty - lengthdir_y(endlen, dir);

			int N = 40;
			for (int i = 0; i < N; i++) {
				float t = i / (float)(N - 1);
				float x = lerp(startx, endx, t);
				float y = lerp(starty, endy, t);
				float off = 10 + 140 * dsin(180.0f * t);
				int N2 = min(1 + 3 * (i / 2), 11);

				if (i % 2 == 0) {
					shoot(N2, x + lengthdir_x(off, dir + 90.0f), y + lengthdir_y(off, dir + 90.0f), dir, spr_bullet_filled, 13);
				} else {
					shoot(N2, x + lengthdir_x(off, dir - 90.0f), y + lengthdir_y(off, dir - 90.0f), dir, spr_bullet_filled, 7);
				}

				wait(4);
			}

			wait(180);
		}

		Repeat (2) {
			slash(self->x, self->y, w->player.x, w->player.y, spr_bullet_arrow, 6);

			wait(50);

			slash(self->x - 50.0f, self->y, w->player.x - 70.0f, w->player.y, spr_bullet_arrow, 2);
			slash(self->x + 50.0f, self->y, w->player.x + 70.0f, w->player.y, spr_bullet_arrow, 2);

			wait(60);
		}
	}
}

static void M_Youmu_Ghost_Sword(mco_coro* co) {
	auto bullet = [](mco_coro* co) {
		// @Note: Object::dir automatically wraps around 360.
		Repeat (360 / 3) {
			self->dir += 3;
			wait(1);
		}

		float target_spd = w->random.rangef(2.50f, 3.50f);
		float acc = 0.05f;
		while (self->spd != target_spd) {
			self->spd = approach(self->spd, target_spd, acc);
			wait(1);
		}
	};

	auto shoot = [&](float x, float y) {
		int N = 18;
		float dir1 = w->random.rangef(0, 360);
		float dir2 = w->random.rangef(0, 360);
		for (int i = 0; i < N; i++) {
			float t = i / (float)N;
			ShootExt(x, y, 1.3f, dir1 + 360 * t, 0, spr_bullet_arrow, 6, 0, bullet);
			ShootExt(x, y, 0.8f, dir2 + 360 * t, 0, spr_bullet_arrow, 6, 0, bullet);
		}
	};

	auto slash = [&](float xstart, float xend, float y) {
		int N = 15;
		for (int i = 0; i < N; i++) {
			float t = i / (float)N;
			float x = lerp(xstart, xend, t);
			shoot(x, y);

			wait(4);
		}
	};

	auto slide_to = [&](float x, float y) {
		LaunchTowardsPoint(self, x, y, 0.02f);
		while (self->spd > 0) {
			wait(1);
		}
	};

	auto speed_up = [&]() {
		float delta_multiplier_start      = w->delta_multiplier;
		float boss_pcb_youmu_effect_start = w->boss_pcb_youmu_effect;

		for (int i = 1; i <= 30; i++) {
			w->delta_multiplier      = lerp(delta_multiplier_start,      1.0f, i / 30.0f);
			w->boss_pcb_youmu_effect = lerp(boss_pcb_youmu_effect_start, 0.0f, i / 30.0f);

			wait(1);
		}

		wait(40);
	};

	slide_to(60, 140);

	while (true) {
		speed_up();

		self->x = PLAY_AREA_W - 60;

		slash(0, PLAY_AREA_W, self->y);
		wait(seconds(2.6f));

		w->boss_pcb_youmu_effect = 1;
		w->delta_multiplier = 0.25f;
		play_sound(snd_kira);

		wait(seconds(2.1f));

		speed_up();

		self->x = 60;

		slash(PLAY_AREA_W, 0, self->y);
		wait(seconds(2.6f));

		w->boss_pcb_youmu_effect = 1;
		w->delta_multiplier = 0.25f;
		play_sound(snd_kira);

		wait(seconds(2.1f));
	}
}

static BossPhase boss_youmu_midboss_phases[] = {
	{
		/* .name   = */ "Nonspell 0",
		/* .hp     = */ 1000,
		/* .time   = */ 60,
		/* .type   = */ PHASE_NONSPELL,
		/* .script = */ M_Youmu_Nonspell_0,
	},
	{
		/* .name   = */ "Ghost Sword \"Fasting of the Young Gaki\"",
		/* .hp     = */ 1000,
		/* .time   = */ 60,
		/* .type   = */ PHASE_SPELLCARD,
		/* .script = */ M_Youmu_Ghost_Sword,
	},
};

static void Youmu_Draw_Spellcard_Background(float delta) {

	{
		Texture* t = GetTexture(tex_pcb_youmu_bg);

		vec2 scale;
		scale.x = (PLAY_AREA_W + 5) / (float)t->width; // floating point thing
		scale.y = (PLAY_AREA_H + 5) / (float)t->height;

		const vec4 color_from = {1,    1, 1, 1};
		const vec4 color_to   = {0.5f, 0, 0, 0.15f};

		vec4 color = lerp(color_from, color_to, w->boss_pcb_youmu_effect);
		color.a *= w->boss_spellcard_background_alpha; // is 1 most of the time

		vec2 pos = {-1, -1}; // floating point thing
		r->draw_texture(t, {}, pos, scale, {}, 0, color);
	}

	{
		Texture* t = GetTexture(tex_pcb_youmu_bg_flowers);

		vec2  pos    = {PLAY_AREA_W / 2.0f, PLAY_AREA_H / 2.0f};
		vec2  scale  = {2, 2};
		vec2  origin = {t->width / 2.0f, t->height / 2.0f};
		float angle  = SDL_GetTicks() / 50.0f;

		vec4 color = color_white;
		color.a = lerp(0.5f, 0.4f, w->boss_pcb_youmu_effect);
		color.a *= w->boss_spellcard_background_alpha; // is 1 most of the time

		r->draw_texture(t, {}, pos, scale, origin, angle, color);
	}
}
