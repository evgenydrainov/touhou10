#include "objects.h"

#include "game.h"
#include "cpml.h"

void player_init(Player* p) {
	*p = {};
	object_init(p, OBJ_TYPE_PLAYER);
	p->x = PLAYER_STARTING_X;
	p->y = PLAYER_STARTING_Y;

	Character* character = p->GetCharacter();
	p->sprite_index = character->spr_idle;
	p->radius = character->radius;
}

void player_update(Player* p, float delta) {
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

	Character* character = p->GetCharacter();

	{
		float spd = (p->focused) ? character->focus_spd : character->move_spd;

		p->hsp = dir_x * spd;
		p->vsp = dir_y * spd;
	}

	Assert(character->shot_type);
	character->shot_type(p, delta);

	{
		if (dir_x < 0.0f) {
			if (p->frame_index < 4.0f) {
				object_animate(p, delta * 2.0f);
			} else {
				object_animate(p, delta);
			}

			if (p->sprite_index != character->spr_left) {
				p->sprite_index = character->spr_left;
				p->frame_index = 0.0f;
			}
		} else if (dir_x > 0.0f) {
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

		p->hitbox_alpha = approach(p->hitbox_alpha,
								   (p->focused) ? 1.0f : 0.0f,
								   0.1f * delta);
	}
}

void player_draw(Player* p, float delta) {
	{
		r->draw_sprite(p->GetSprite(), (int)p->frame_index, {p->x, p->y});
	}

	if (p->hitbox_alpha > 0.0f) {
		float angle = (float)SDL_GetTicks() / 20.0f;
		glm::vec4 color = {1.0f, 1.0f, 1.0f, p->hitbox_alpha};
		r->draw_sprite(GetSprite(spr_player_hitbox), 0, {p->x, p->y}, {1.0f, 1.0f}, angle, color);
	}
}
