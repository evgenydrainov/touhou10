#include "objects.h"

#include "game.h"
#include "world.h"
#include "input.h"

#define POINT_OF_COLLECTION 96.0f

void player_init(Player* p) {
	*p = {};
	object_init(p, OBJ_TYPE_PLAYER);
	p->x = PLAYER_STARTING_X;
	p->y = PLAYER_STARTING_Y;

	Character* character = p->GetCharacter();
	p->sprite_index = character->spr_idle;
	p->radius = character->radius;

	p->iframes = PLAYER_RESPAWN_IFRAMES;
	/*if (from_death)*/ {
		p->state = PLAYER_STATE_APPEARING;
		p->timer = PLAYER_APPEAR_TIME;
	}
}

static void player_animate(Player* p, float delta) {
	Character* character = p->GetCharacter();

	if (p->hsp < 0.0f) {
		if (p->frame_index < 4.0f) {
			p->frame_index = object_animate(p->sprite_index, p->frame_index, delta * 2.0f);
		} else {
			p->frame_index = object_animate(p->sprite_index, p->frame_index, delta);
		}

		if (p->sprite_index != character->spr_left) {
			p->sprite_index = character->spr_left;
			p->frame_index = 0.0f;
		}
	} else if (p->hsp > 0.0f) {
		if (p->frame_index < 4.0f) {
			p->frame_index = object_animate(p->sprite_index, p->frame_index, delta * 2.0f);
		} else {
			p->frame_index = object_animate(p->sprite_index, p->frame_index, delta);
		}

		if (p->sprite_index != character->spr_right) {
			p->sprite_index = character->spr_right;
			p->frame_index = 0.0f;
		}
	} else {
		if (p->sprite_index == character->spr_left || p->sprite_index == character->spr_right) {
			p->frame_index -= p->GetSprite().anim_spd * delta;
			p->frame_index = fminf(p->frame_index, 3.0f);
			if (p->frame_index < 0.0f) {
				p->sprite_index = character->spr_idle;
				p->frame_index = 0.0f;
			}
		} else {
			p->frame_index = object_animate(p->sprite_index, p->frame_index, delta);
		}
	}
}

void player_update(Player* p, float delta) {
	Character* character = p->GetCharacter();

	p->hsp = 0;
	p->vsp = 0;
	p->focused = false;

	auto try_use_bomb = [&]() {
		if (is_input_pressed(INPUT_BOMB)) {
			if (p->bomb_timer <= 0) {
				if (game.stats.bombs > 0) {
					if (character->bomb) {
						character->bomb(p);
					}

					game.stats.bombs--;
					p->bomb_timer = PLAYER_BOMB_TIME;

					// Deathbomb
					p->state = PLAYER_STATE_NORMAL;
					p->iframes = PLAYER_RESPAWN_IFRAMES;

					return true;
				}
			}
		}

		return false;
	};

	switch (p->state) {
		case PLAYER_STATE_NORMAL: {
			vec2 dir = {};

			if (is_input_held(INPUT_MOVE_UP)) {
				dir.y -= 1.0f;
			}
			if (is_input_held(INPUT_MOVE_DOWN)) {
				dir.y += 1.0f;
			}
			if (is_input_held(INPUT_MOVE_LEFT)) {
				dir.x -= 1.0f;
			}
			if (is_input_held(INPUT_MOVE_RIGHT)) {
				dir.x += 1.0f;
			}

			p->focused = is_input_held(INPUT_FOCUS);

			dir = normalize0(dir);

			{
				float spd = (p->focused) ? character->focus_spd : character->move_spd;

				p->hsp = dir.x * spd;
				p->vsp = dir.y * spd;
			}

			if (character->shot_type) {
				character->shot_type(p, delta);
			}

			try_use_bomb();

			if (p->iframes > 0) {
				p->iframes -= delta;
			}

			player_animate(p, delta);

			if (p->y < POINT_OF_COLLECTION) {
				For (it, world.pickups) {
					it->homing_target = p->id;
				}
			}
			break;
		}

		case PLAYER_STATE_DYING: {
			// Deathbomb
			if ((PLAYER_DEATH_TIME - p->timer) < character->deathbomb_time) {
				if (try_use_bomb()) {
					break;
				}
			}

			p->timer -= delta;
			if (p->timer <= 0) {
				if (game.stats.lives > 0) {
					game.stats.lives--;

					int drop = min(game.stats.power, 16);
					game.stats.power -= drop;
					drop = min(drop, 12);

					while (drop > 0) {
						PickupType type;
						if (drop >= 8) {
							drop -= 8;
							type = PICKUP_TYPE_POWER_BIG;
						} else {
							drop--;
							type = PICKUP_TYPE_POWER;
						}
						float x = p->x + random_rangef(&world.rng, -50.0f, 50.0f);
						float y = p->y + random_rangef(&world.rng, -50.0f, 50.0f);
						drop_pickup(x, y, type);
					}
				} else {
					// GameOver();
					drop_pickup(p->x, p->y, PICKUP_TYPE_FULL_POWER);
				}

				world.death_effect = {};
				world.death_effect.show = true;
				world.death_effect.x = p->x;
				world.death_effect.y = p->y;

				object_cleanup(p);
				player_init(p);
				return;
			}
			break;
		}

		case PLAYER_STATE_APPEARING: {
			p->timer -= delta;
			if (p->timer <= 0) {
				p->state = PLAYER_STATE_NORMAL;
			}
			break;
		}
	}

	if (p->bomb_timer > 0) {
		p->bomb_timer -= delta;
	}

	p->hitbox_alpha = approach(p->hitbox_alpha,
							   (p->focused) ? 1.0f : 0.0f,
							   0.1f * delta);
}

bool player_get_hit(Player* p) {
	if (p->state == PLAYER_STATE_NORMAL) {
		if (p->iframes <= 0) {
			p->state = PLAYER_STATE_DYING;
			p->timer = PLAYER_DEATH_TIME;

			play_sound(get_sound(snd_pichuun));

			return true;
		}
	}

	return false;
}

void player_draw(Player* p, float delta) {
	{
		vec4 color = color_white;
		vec2 scale = {1, 1};

		if (p->state == PLAYER_STATE_DYING || p->state == PLAYER_STATE_APPEARING) {
			float f;
			if (p->state == PLAYER_STATE_DYING) {
				f = 1.0f - p->timer / PLAYER_DEATH_TIME;
			} else {
				f = p->timer / PLAYER_APPEAR_TIME;
			}

			scale.x = lerp(1.0f, 0.25f, f);
			scale.y = lerp(1.0f, 2.0f, f);

			color.a = lerp(1.0f, 0.0f, f);
		} else {
			if (p->iframes > 0.0f) {
				if (SDL_GetTicks() % 200 >= 100) {
					color = {1, 1, 1, 0.5f};
				}
			}
		}

		draw_sprite(p->GetSprite(), (int)p->frame_index, {p->x, p->y}, scale, 0, color);
	}

	if (p->character_index == CHARACTER_REIMU) {
		vec4 color = {1, 1, 1, 0.50f};

		float angle1 = SDL_GetTicks() / 5.0f;
		float angle2 = 90 - angle1;

		float f = min(p->hitbox_alpha * 1.5f, 1.0f);

		float dir1 = lerp(0.0f, 70.0f, f);
		float dir2 = 180 - dir1;

		float orb_x1 = p->x + lengthdir_x(26, dir1);
		float orb_x2 = p->x + lengthdir_x(26, dir2);

		float orb_y1 = p->y + lengthdir_y(26, dir1);
		float orb_y2 = p->y + lengthdir_y(26, dir2);

		draw_sprite(get_sprite(spr_reimu_orb), 1, {orb_x1, orb_y1}, {1, 1}, angle1, color);
		draw_sprite(get_sprite(spr_reimu_orb), 1, {orb_x2, orb_y2}, {1, 1}, angle2, color);
	}

	if (p->hitbox_alpha > 0.0f) {
		float angle = SDL_GetTicks() / 20.0f;
		vec4 color = {1.0f, 1.0f, 1.0f, p->hitbox_alpha};
		draw_sprite(get_sprite(spr_player_hitbox), 0, {p->x, p->y}, {1.0f, 1.0f}, angle, color);
	}
}
