#pragma once

static void M_Youmu_Nonspell_0(mco_coro* co) {

	auto shoot = [&](float x, float y, float dir, u32 sprite_index, int frame_index) {
		int N = 8;
		for (int i = 0; i < N; i++) {
			float t = i / (float)(N - 1);
			ShootExt(self, x, y, 1, dir - 90.0f + 180.0f * t, 0, sprite_index, frame_index);
		}
	};

	auto slash = [&](float start_x, float start_y, float target_x, float target_y, u32 sprite_index, int frame_index) {
		int N = 12;
		for (int i = 0; i < N; i++) {
			float x = start_x + 20.0f * (i - N / 2.0f);
			float y = start_y;

			for (int i = 1; i <= 3; i++) {
				ShootExt(self, x, y, 1.0f + i, point_direction(x, y, target_x, target_y), 0, sprite_index, frame_index);
			}

			Wait(co, 1);
		}
	};

	while (true) {
		Repeat (2) {
			float dir = DirToPlayer(self);
			float startlen = 180;
			float startx = self->x + lengthdir_x(startlen, dir);
			float starty = self->y + lengthdir_y(startlen, dir);
			float endlen = 380;
			float endx = startx - lengthdir_x(endlen, dir);
			float endy = starty - lengthdir_y(endlen, dir);

			int N = 40;
			for (int i = 0; i < N; i++) {
				float t = i / (float)(N - 1);
				float x = lerp(startx, endx, t);
				float y = lerp(starty, endy, t);
				float off = 30.0f + 150.0f * sinf(glm::radians(180.0f * t));

				if (i % 2 == 0) {
					shoot(x + lengthdir_x(off, dir + 90.0f), y + lengthdir_y(off, dir + 90.0f), dir, spr_bullet_filled, 13);
				} else {
					shoot(x + lengthdir_x(off, dir - 90.0f), y + lengthdir_y(off, dir - 90.0f), dir, spr_bullet_filled, 7);
				}

				Wait(co, 4);
			}

			Wait(co, 180);
		}

		Repeat (2) {
			slash(self->x, self->y, w->player.x, w->player.y, spr_bullet_arrow, 6);

			Wait(co, 50);

			slash(self->x - 50.0f, self->y, w->player.x - 70.0f, w->player.y, spr_bullet_arrow, 2);
			slash(self->x + 50.0f, self->y, w->player.x + 70.0f, w->player.y, spr_bullet_arrow, 2);

			Wait(co, 60);
		}
	}
}

static void M_Youmu_Ghost_Sword(mco_coro* co) {
	
	auto bullet = [](mco_coro* co) {
		Repeat (360 / 5) {
			self->dir += 5;
			Wait(co, 1);
		}

		while (self->spd < 2.5f) {
			self->spd += 0.1f;
			Wait(co, 1);
		}
	};

	auto shoot = [&](float x, float y, float dir) {
		int N = 25;
		for (int i = 0; i < N; i++) {
			ShootExt(self, x, y, 1, dir + 360.0f * (i / (float)N), 0, spr_bullet_arrow, 6, 0, bullet);
		}
	};

	auto slash = [&](float xstart, float xend, float y) {
		int N = 15;
		for (int i = 0; i < N; i++) {
			float t = i / (float)(N - 1);
			float x = lerp(xstart, xend, t);
			shoot(x, y, point_direction(x, y, w->player.x, w->player.y));

			Wait(co, 5);
		}
	};

	auto slide_to = [&](float x, float y) {
		LaunchTowardsPoint(self, x, y, 0.02f);
		while (self->spd > 0) {
			Wait(co, 1);
		}
	};

	auto change_delta = [&](float delta, float spd) {
		// while (w->gameplay_delta != delta) {
		// 	w->gameplay_delta = approach(w->gameplay_delta, delta, spd);
		// 	Wait(co, 1);
		// }

		Wait(co, 60);
	};

	slide_to(60, 140);

	while (true) {
		change_delta(1, 1.0f / 60.0f);

		self->x = 384 - 60;

		slash(0, self->x, self->y);

		Wait(co, 120);

		// w->gameplay_delta = 0.25f;

		Wait(co, 120);

		change_delta(1, 1.0f / 60.0f);

		self->x = 60;

		slash(384, self->x, self->y);

		Wait(co, 120);

		// w->gameplay_delta = 0.25f;

		Wait(co, 120);
	}
}

static BossPhase midboss_youmu_phases[] = {
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
