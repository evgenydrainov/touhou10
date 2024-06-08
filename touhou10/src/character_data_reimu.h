#pragma once

static void reimu_shot_type(Player* p, float delta) {

	auto reimu_shoot_card = [](float x, float y, float dir, float dmg) {
		PlayerBullet b = {};
		object_init(&b, OBJ_TYPE_PLAYER_BULLET);

		b.x = x;
		b.y = y;
		b.spd = 16.0f;
		b.dir = dir;
		b.radius = 12.0f;
		b.sprite_index = spr_reimu_shot_card;
		b.dmg = dmg;
		b.type = PLAYER_BULLET_REIMU_CARD;

		w->p_bullets.add(b);
	};

	auto reimu_shoot_orb = [](float x, float y, float dir, float dmg) {
		PlayerBullet b = {};
		object_init(&b, OBJ_TYPE_PLAYER_BULLET);

		b.x = x;
		b.y = y;
		b.spd = 12.0f;
		b.dir = dir;
		b.radius = 12.0f;
		b.sprite_index = spr_reimu_shot_orb;
		b.dmg = dmg;
		b.type = PLAYER_BULLET_REIMU_ORB_SHOT;

		w->p_bullets.add(b);
	};

	p->reimu.fire_timer += delta;

	while (p->reimu.fire_timer >= 4.0f) {
		if (p->reimu.fire_queue == 0) {
			if (is_key_held(SDL_SCANCODE_Z)) {
				p->reimu.fire_queue = 8;
			}
		}

		if (p->reimu.fire_queue > 0) {
			int card_shot_type = 0;
			int orb_shot_type = 0;

			int power = g->stats.power;

			if (power >= 128) {
				card_shot_type = 3;
			} else if (power >= 32) {
				card_shot_type = 2;
			} else if (power >= 8) {
				card_shot_type = 1;
			}

			if (power >= 128) {
				orb_shot_type = 3;
			} else if (power >= 80) {
				orb_shot_type = 2;
			} else if (power >= 48) {
				orb_shot_type = 1;
			}

			float min_dps = 75.0f;
			float max_dps = 150.0f;
			float dps = lerp(min_dps, max_dps, (float)power / (float)MAX_POWER);

			{
				float card_fraction = 2.0f / 3.0f;
				float card_dps = dps * card_fraction;
				float shots_per_sec = 15.0f;

				switch (card_shot_type) {
					case 0: {
						int shot_count = 1;
						float card_dmg = card_dps / shots_per_sec / (float)shot_count;

						reimu_shoot_card(p->x, p->y - 10.0f, 90.0f, card_dmg);
						break;
					}
					case 1: {
						int shot_count = 2;
						float card_dmg = card_dps / shots_per_sec / (float)shot_count;

						for (int i = 0; i < shot_count; i++) {
							reimu_shoot_card(p->x - 8.0f + (float)i * 16.0f, p->y - 10.0f, 90.0f, card_dmg);
						}
						break;
					}
					case 2: {
						int shot_count = 3;
						float card_dmg = card_dps / shots_per_sec / (float)shot_count;

						for (int i = 0; i < shot_count; i++) {
							reimu_shoot_card(p->x, p->y - 10.0f, 90.0f - 5.0f + (float)i * 5.0f, card_dmg);
						}
						break;
					}
					case 3: {
						int shot_count = 4;
						float card_dmg = card_dps / shots_per_sec / (float)shot_count;

						for (int i = 0; i < shot_count; i++) {
							reimu_shoot_card(p->x, p->y - 10.0f, 90.0f - 7.5f + (float)i * 5.0f, card_dmg);
						}
						break;
					}
				}
			}

			{
				int frame = 8 - p->reimu.fire_queue;
				float orb_fraction = 1.0f / 3.0f;
				float orb_dps = dps * orb_fraction;

				switch (orb_shot_type) {
					case 0: {
						int shot_count = 2;
						float shots_per_sec = 60.0f / (4.0f * 4.0f);
						float orb_dmg = orb_dps / shots_per_sec / (float)shot_count;

						if (frame % 4 == 0) {
							for (int i = 0; i < shot_count; i++) {
								reimu_shoot_orb(p->x, p->y, 90.0f + 70.0f * ((i == 0) ? -1.0f : 1.0f), orb_dmg);
							}
						}
						break;
					}
					case 1: {
						int shot_count = 4;
						float shots_per_sec = 60.0f / (4.0f * 4.0f);
						float orb_dmg = orb_dps / shots_per_sec / (float)shot_count;

						if (frame % 4 == 0) {
							for (int i = 0; i < 2; i++) {
								reimu_shoot_orb(p->x, p->y, 90.0f + 50.0f * ((i == 0) ? -1.0f : 1.0f), orb_dmg);
								reimu_shoot_orb(p->x, p->y, 90.0f + 70.0f * ((i == 0) ? -1.0f : 1.0f), orb_dmg);
							}
						}
						break;
					}
					case 2: {
						int shot_count = 2;
						float shots_per_sec = 15.0f;
						float orb_dmg = orb_dps / shots_per_sec / (float)shot_count;

						float off = 45.0f + 15.0f * (float)(frame % 3);

						for (int i = 0; i < shot_count; i++) {
							reimu_shoot_orb(p->x, p->y, 90.0f + ((i == 0) ? -off : off), orb_dmg);
						}
						break;
					}
					case 3: {
						int shot_count = 2;
						float shots_per_sec = 15.0f;
						float orb_dmg = orb_dps / shots_per_sec / (float)shot_count;

						float off = 30.0f + 15.0f * (float)(frame % 4);

						for (int i = 0; i < shot_count; i++) {
							reimu_shoot_orb(p->x, p->y, 90.0f + ((i == 0) ? -off : off), orb_dmg);
						}
						break;
					}
				}
			}

			//PlaySound("se_plst00.wav");
			p->reimu.fire_queue--;
		}

		p->reimu.fire_timer -= 4.0f;
	}
}

static void reimu_bomb(Player* p) {

}
