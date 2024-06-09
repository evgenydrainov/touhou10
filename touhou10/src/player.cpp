#include "objects.h"

#include "game.h"
#include "cpml.h"

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
			object_animate(p, delta * 2.0f);
		} else {
			object_animate(p, delta);
		}

		if (p->sprite_index != character->spr_left) {
			p->sprite_index = character->spr_left;
			p->frame_index = 0.0f;
		}
	} else if (p->hsp > 0.0f) {
		if (p->frame_index < 4.0f) {
			object_animate(p, delta * 2.0f);
		} else {
			object_animate(p, delta);
		}

		if (p->sprite_index != character->spr_right) {
			p->sprite_index = character->spr_right;
			p->frame_index = 0.0f;
		}
	} else {
		if (p->sprite_index == character->spr_left || p->sprite_index == character->spr_right) {
			p->frame_index -= p->GetSprite()->anim_spd * delta;
			p->frame_index = fminf(p->frame_index, 3.0f);
			if (p->frame_index < 0.0f) {
				p->sprite_index = character->spr_idle;
				p->frame_index = 0.0f;
			}
		} else {
			object_animate(p, delta);
		}
	}
}

void player_update(Player* p, float delta) {
	Character* character = p->GetCharacter();

	p->hsp = 0;
	p->vsp = 0;
	p->focused = false;

	auto try_use_bomb = [&]() {
		if (is_key_pressed(SDL_SCANCODE_X)) {
			if (p->bomb_timer <= 0) {
				if (g->stats.bombs > 0) {
					Assert(character->bomb);
					character->bomb(p);

					g->stats.bombs--;
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
			float dir_x = 0;
			float dir_y = 0;

			if (is_key_held(SDL_SCANCODE_UP)) {
				dir_y -= 1.0f;
			}
			if (is_key_held(SDL_SCANCODE_DOWN)) {
				dir_y += 1.0f;
			}
			if (is_key_held(SDL_SCANCODE_LEFT)) {
				dir_x -= 1.0f;
			}
			if (is_key_held(SDL_SCANCODE_RIGHT)) {
				dir_x += 1.0f;
			}

			p->focused = is_key_held(SDL_SCANCODE_LSHIFT);

			normalize0(&dir_x, &dir_y);

			{
				float spd = (p->focused) ? character->focus_spd : character->move_spd;

				p->hsp = dir_x * spd;
				p->vsp = dir_y * spd;
			}

			Assert(character->shot_type);
			character->shot_type(p, delta);

			try_use_bomb();

			if (p->iframes > 0) {
				p->iframes -= delta;
			}

			player_animate(p, delta);

			if (p->y < POINT_OF_COLLECTION) {
				For (it, w->pickups) {
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
				if (g->stats.lives > 0) {
					g->stats.lives--;

					int drop = min(g->stats.power, 16);
					g->stats.power -= drop;
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
						float x = p->x + w->random.rangef(-50.0f, 50.0f);
						float y = p->y + w->random.rangef(-50.0f, 50.0f);
						drop_pickup(x, y, type);
					}
				} else {
					// GameOver();
					drop_pickup(p->x, p->y, PICKUP_TYPE_FULL_POWER);
				}

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

void player_draw(Player* p, float delta) {
	{
		glm::vec4 color = color_white;
		glm::vec2 scale = {1, 1};

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

		r->draw_sprite(p->GetSprite(), (int)p->frame_index, {p->x, p->y}, scale, 0, color);
	}

	if (p->character_index == CHARACTER_REIMU) {
		glm::vec4 color = {1, 1, 1, 0.50f};

		float angle1 = SDL_GetTicks() / 5.0f;
		float angle2 = 90 - angle1;

		float f = min(p->hitbox_alpha * 1.5f, 1.0f);

		float dir1 = lerp(0.0f, 70.0f, f);
		float dir2 = 180 - dir1;

		float orb_x1 = p->x + lengthdir_x(26, dir1);
		float orb_x2 = p->x + lengthdir_x(26, dir2);

		float orb_y1 = p->y + lengthdir_y(26, dir1);
		float orb_y2 = p->y + lengthdir_y(26, dir2);

		r->draw_sprite(GetSprite(spr_reimu_orb), 1, {orb_x1, orb_y1}, {1, 1}, angle1, color);
		r->draw_sprite(GetSprite(spr_reimu_orb), 1, {orb_x2, orb_y2}, {1, 1}, angle2, color);
	}

	if (p->hitbox_alpha > 0.0f) {
		float angle = SDL_GetTicks() / 20.0f;
		glm::vec4 color = {1.0f, 1.0f, 1.0f, p->hitbox_alpha};
		r->draw_sprite(GetSprite(spr_player_hitbox), 0, {p->x, p->y}, {1.0f, 1.0f}, angle, color);
	}
}
