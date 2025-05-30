#pragma once

static void Cirno_Draw_Spellcard_Background(float delta) {
	const Texture& t = get_texture(tex_cirno_spellcard_background);

	vec4 color = {1, 1, 1, world.boss_spellcard_background_alpha};
	float scale = PLAY_AREA_W / (float)t.width;

	int y = ((SDL_GetTicks() / 50) % PLAY_AREA_W) - PLAY_AREA_W;

	draw_texture(t, {}, {0, (float)y}, {scale, scale}, {}, 0, color);
	y += PLAY_AREA_W;

	draw_texture(t, {}, {0, (float)y}, {scale, scale}, {}, 0, color);
	y += PLAY_AREA_W;

	draw_texture(t, {}, {0, (float)y}, {scale, scale}, {}, 0, color);
	y += PLAY_AREA_W;
}

#if 1
static void Cirno_Nonspell_0(mco_coro* co) {
	auto shoot_radial_bullets = [&]() {
		ShootRadial(17, 360.0f / 17.0f, [&]() {
			return Shoot(3.5f, DirToPlayer(self), 0.0f, spr_bullet_outline, 6);
		});
	};

	while (true) {
		Repeat (3) {
			for (int i = 0; i < 6; i++) {
				ShootRadial(7 - i, 5.0f, [&]() {
					return Shoot(lerp(4.0f, 7.5f, (float)i / 5.0f), DirToPlayer(self), 0.0f, spr_bullet_pellet, 6);
				});
			}

			wait(60);
		}

		wait(15);

		Wander(self);

		wait(15);

		for (int i = 0; i < 3; i++) {
			shoot_radial_bullets();

			wait(15);

			ShootRadial(17, 360.0f / 17.0f, [&]() {
				return Shoot(4.5f, DirToPlayer(self), -0.08f, spr_bullet_pellet, 15, 0, [](mco_coro* co) {
					while (self->spd > 0.0f) {
						wait(1);
					}
					self->spd = 5.0f;
					self->dir = DirToPlayer(self);
					self->acc = 0.0f;
				});
			});

			wait(15);

			shoot_radial_bullets();

			wait(60);

			if (i == 2) {
				GoBack(self);
			} else {
				Wander(self);
			}
		}

		wait(60);
	}
}
#elif 1
static void Cirno_Nonspell_0(mco_coro* co) {
	u32 sprite_index[] = {
		spr_bullet_arrow,
		spr_bullet_outline,
		spr_bullet_filled,
		spr_bullet_rice,
		spr_bullet_kunai,
		spr_bullet_shard,
		spr_bullet_card,
		spr_bullet_bullet,
	};

	float dir = 0.0f;
	float acc = 0.0f;

	while (true) {
		ShootRadial(5, 360.0f / 5.0f, [&]() {
			return Shoot(random_rangef(&world.rng, 2.0f, 3.0f), dir, 0.0f, sprite_index[world.random.next() % ArrayLength(sprite_index)], world.random.next() % 16, 0, [](mco_coro* co) {
				for (;;) {
					self->dir += 2;
					wait(1);
				}
			});
		});

		wait(1);

		dir += acc;
		acc += 0.1f;

		// self->y += 0.1f;
	}
}
#else
static void Cirno_Nonspell_0(mco_coro* co) {
	while (true) {
		ArenaClear(&world.temp_arena_for_boss);

		ShootRadialA(5, 10, [&]() {
			return ShootRadialA(2, 1, [&]() {
				return Shoot(3, DirToPlayer(self), 0, spr_bullet_pellet, 6);
			});
		});

		wait(60);
	}
}
#endif

static void Cirno_Icicle_Fall(mco_coro* co) {
	while (true) {
		for (int i = 0; i < 11; i++) {
			float off = 100.0f - 5.0f * i;

			for (int side = 0; side < 2; side++) {
				float dir = (side == 0) ? (270.0f - off) : (270.0f + off);

				for (int j = 0; j < 3; j++) {
					float target_x = self->x + lengthdir_x(100.0f + 90.0f * j, dir);
					float target_y = self->y + lengthdir_y(100.0f + 90.0f * j, dir);

					Bullet* bullet = Shoot(0, dir, 0, spr_bullet_pellet, 6, 0, [](mco_coro* co) {
						wait(50);
						self->spd = 2;
						self->acc = 0;
						if (90 <= self->dir && self->dir < 270) {
							self->dir += 90;
						} else {
							self->dir -= 90;
						}
					});

					LaunchTowardsPoint(bullet, target_x, target_y, 0.07f);
				}
			}

			if (i % 3 == 2) {
				ShootRadial(5, 20, [&]() {
					return Shoot(2, DirToPlayer(self), 0, spr_bullet_filled, 13);
				});
			}

			wait(20);
		}
	}
}

static void Cirno_Nonspell_1(mco_coro* co) {
	while (true) {
		Wander(self);

		for (int i = 1; i <= 16; i++) {
			if (i % 2 == 0) {
				ShootRadial(8, 360.0f / 8.0f, [&]() {
					return Shoot(4, DirToPlayer(self) + 360.0f / 8.0f / 2.0f, 0, spr_bullet_outline, 6);
				});
			} else {
				ShootRadial(8, 360.0f / 8.0f, [&]() {
					return Shoot(2, DirToPlayer(self), 0, spr_bullet_small, 6);
				});
			}

			wait(10);
		}

		Wander(self);

		for (int i = 1; i <= 3; i++) {
			ShootRadial(3, 20, [&]() {
				return ShootLazer(3.5, DirToPlayer(self), 180, 2, 15);
			});

			wait(60);
		}
	}
}

static void Cirno_Perfect_Freeze(mco_coro* co) {
	while (true) {
		instance_id bullets[200] = {};
		int nbullets = 0;

		Wander(self);

		Repeat (100) {
			Repeat (2) {
				int colors[] = {2, 6, 10, 13, 14};
				int color = random_choose(&world.rng, colors);
				bullets[nbullets++] = Shoot(random_rangef(&world.rng, 1.0f, 4.0f), random_rangef(&world.rng, 0.0f, 360.0f), 0, spr_bullet_outline, color)->id;
			}

			wait(1);
		}

		wait(60);

		for (instance_id id : bullets) {
			if (Bullet* b = world.find_bullet(id)) {
				b->spd = 0;
				b->frame_index = 15;
			}
		}
		Kira(self);

		wait(60);

		Wander(self);

		Repeat (5) {
			for (int i = 0; i < 5; i++) {
				ShootRadial(4, 30, [&]() {
					return Shoot(lerp(2.0f, 6.0f, i / 4.0f), DirToPlayer(self), 0, spr_bullet_outline, 6);
				});
			}

			wait(10);
		}

		wait(60);

		for (instance_id id : bullets) {
			if (Bullet* b = world.find_bullet(id)) {
				b->spd = 0;
				b->dir = random_rangef(&world.rng, 0.0f, 360.0f);
				b->acc = random_rangef(&world.rng, 0.01f, 0.015f);
			}
		}
		Kira(self);

		wait(180);
	}
}

static void Cirno_Diamond_Blizzard(mco_coro* co) {
	while (true) {
		Repeat (30) {
			instance_id bullets[6] = {};

			float x = self->x + random_rangef(&world.rng, -50.0f, 50.0f);
			float y = self->y + random_rangef(&world.rng, -50.0f, 50.0f);

			int n = random_range(&world.rng, 4, 7);
			for (int i = 0; i < n; i++) {
				bullets[i] = ShootExt(x, y, random_rangef(&world.rng, 4.0f, 5.0f), random_rangef(&world.rng, 0.0f, 360.0f), 0, spr_bullet_pellet, 6)->id;
			}

			wait(4);

			for (int i = 0; i < n; i++) {
				if (Bullet* b = world.find_bullet(bullets[i])) {
					b->spd = random_rangef(&world.rng, 1.0f, 2.0f);
				}
			}
		}

		Wander(self);
	}
}

static BossPhase boss_cirno_phases[] = {
	{
		/* .name   = */ "Nonspell 0",
		/* .hp     = */ 1000,
		/* .time   = */ 60,
		/* .type   = */ PHASE_NONSPELL,
		/* .script = */ Cirno_Nonspell_0,
	},
	{
		/* .name   = */ "Ice Sign \"Icicle Fall\"",
		/* .hp     = */ 1000,
		/* .time   = */ 60,
		/* .type   = */ PHASE_SPELLCARD,
		/* .script = */ Cirno_Icicle_Fall,
	},
	{
		/* .name   = */ "Nonspell 1",
		/* .hp     = */ 1000,
		/* .time   = */ 60,
		/* .type   = */ PHASE_NONSPELL,
		/* .script = */ Cirno_Nonspell_1,
	},
	{
		/* .name   = */ "Freeze Sign \"Perfect Freeze\"",
		/* .hp     = */ 1000,
		/* .time   = */ 60,
		/* .type   = */ PHASE_SPELLCARD,
		/* .script = */ Cirno_Perfect_Freeze,
	},
	{
		/* .name   = */ "Snow Sign \"Diamond Blizzard\"",
		/* .hp     = */ 1000,
		/* .time   = */ 60,
		/* .type   = */ PHASE_SPELLCARD,
		/* .script = */ Cirno_Diamond_Blizzard,
	},
};
