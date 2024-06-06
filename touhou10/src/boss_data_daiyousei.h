#pragma once

#if 1
static void Daiyousei_Nonspell_0(mco_coro* co) {

	auto tp = [&]() {
		self->x = w->random.range(32.0f, PLAY_AREA_W - 32.0f);
		self->y = w->random.range(32.0f, BOSS_STARTING_Y * 2.0f - 32.0f);
	};

	while (true) {
		for (int j = 1; j <= 2; j++) {
			int n = 48;

			for (int i = 0; i < n; i++) {
				float dir;
				int color;
				if (j == 1) {
					dir = lerp(270.0f + 360.0f, 270.0f, i / (float)n);
					color = 10;
				} else {
					dir = lerp(270.0f, 270.0f + 360.0f, i / (float)n);
					color = 2;
				}

				float spd = lerp(1.5f, 3.0f, i / (float)n);
				float a   = lerp(1.1f, 1.5f, i / (float)n);

				Shoot(self, spd,     dir, 0, spr_bullet_kunai, color);
				Shoot(self, spd * a, dir, 0, spr_bullet_kunai, color);

				Wait(co, 1);
			}

			Wait(co, 100);

			tp();

			Wait(co, 100);
		}

		Repeat (20) {
			ShootRadial(3, 40, [&]() {
				return Shoot(self, 2.25f, DirToPlayer(self), 0, spr_bullet_pellet, 6);
			});

			/*
			ShootRadialArr<2>(3, 40, [&]() {
				auto script = [](mco_coro* co) {
					Wait(co, 30);
					self->spd = 4;
					self->acc = 0;
					self->dir = DirToPlayer(self);
				};

				return std::array<Bullet*, 2> {
					Shoot(self, 4,        DirToPlayer(self), -0.1f, spr_bullet_pellet, 15, 0, script),
					Shoot(self, 4 * 1.2f, DirToPlayer(self), -0.1f, spr_bullet_pellet, 15, 0, script),
				};
			});
			*/

			auto script = [](mco_coro* co) {
				Wait(co, 30);
				self->spd = 4;
				self->acc = 0;
				self->dir = DirToPlayer(self);
			};

			ShootRadial(3, 40, [&]() {
				return Shoot(self, 4, DirToPlayer(self), -0.1f, spr_bullet_pellet, 15, 0, script);
			});

			ShootRadial(3, 40, [&]() {
				return Shoot(self, 4 * 1.2f, DirToPlayer(self), -0.1f, spr_bullet_pellet, 15, 0, script);
			});

			Wait(co, 10);
		}

		Wait(co, 100);

		tp();

		Wait(co, 100);
	}
}
#else
static void Daiyousei_Nonspell_0(mco_coro* co) {
	check_stack_usage(co);
}
#endif

static BossPhase boss_daiyousei_phases[] = {
	{
		/* .name   = */ "Nonspell 0",
		/* .hp     = */ 1000,
		/* .time   = */ 60,
		/* .type   = */ PHASE_NONSPELL,
		/* .script = */ Daiyousei_Nonspell_0,
	},
};
