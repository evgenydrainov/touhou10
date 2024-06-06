#include "world.h"

#include "game.h"
#include "cpml.h"

#include <glad/gl.h>
#include <glm/gtc/matrix_transform.hpp>
#include <stb/stb_sprintf.h>

World* w;

void World::init() {
	player_init(&player);

	boss.flags |= FLAG_INSTANCE_DEAD;

	bullets   = array_from_arena<Bullet>(&g->arena, MAX_BULLETS);
	p_bullets = array_from_arena<PlayerBullet>(&g->arena, MAX_PLAYER_BULLETS);

	{
		mco_desc desc = mco_desc_init(GetStageData(stage_index)->script, 0);
		mco_create(&co, &desc);
	}
}

void World::destroy() {

	if (co) {
		mco_destroy(co);
		co = nullptr;
	}

	For (b, p_bullets) {
		object_cleanup(b);
	}
	p_bullets.clear();

	For (b, bullets) {
		object_cleanup(b);
	}
	bullets.clear();

	if (!(boss.flags & FLAG_INSTANCE_DEAD)) {
		object_cleanup(&boss);
	}
	boss.flags |= FLAG_INSTANCE_DEAD;

	object_cleanup(&player);
}

void World::physics_update(float delta) {
	player.x += player.hsp * delta;
	player.y += player.vsp * delta;

	boss.x += boss.spd * cosf(glm::radians(boss.dir)) * delta;
	boss.y -= boss.spd * sinf(glm::radians(boss.dir)) * delta;
	boss.spd += boss.acc * delta;
	boss.spd = max(boss.spd, 0.0f);

	For (b, bullets) {
		b->x += b->spd * cosf(glm::radians(b->dir)) * delta;
		b->y -= b->spd * sinf(glm::radians(b->dir)) * delta;
		b->spd += b->acc * delta;
		b->spd = max(b->spd, 0.0f);
	}

	For (b, p_bullets) {
		b->x += b->spd * cosf(glm::radians(b->dir)) * delta;
		b->y -= b->spd * sinf(glm::radians(b->dir)) * delta;
	}

	// 
	// Check collisions.
	// 

	For (b, bullets) {
		if (circle_vs_circle(player.x, player.y, player.radius, b->x, b->y, b->radius)) {
			player_init(&player);

			object_cleanup(b);
			Remove(b, bullets);
		}
	}

	if (!(boss.flags & FLAG_INSTANCE_DEAD)) {
		For (b, p_bullets) {
			if (circle_vs_circle(boss.x, boss.y, boss.radius, b->x, b->y, b->radius)) {
				if (boss.state == BOSS_STATE_NORMAL) {
					boss.hp -= b->dmg;
				}

				object_cleanup(b);
				Remove(b, p_bullets);
			}
		}
	}
}

static bool out_of_bounds(float x, float y, float off = 50.0f) {
	if (x < -off || y < -off || x > PLAY_AREA_W + off || y > PLAY_AREA_H + off) {
		return true;
	}
	return false;
}

void World::update(float delta) {

	// Update
	{
		player_update(&player, delta);

		if (!(boss.flags & FLAG_INSTANCE_DEAD)) {
			boss_update(&boss, delta);
		}

		For (b, p_bullets) {
			switch (b->type) {
				case PLAYER_BULLET_REIMU_ORB_SHOT: {

					auto find_target = [&]() -> Object* {
						if (!(boss.flags & FLAG_INSTANCE_DEAD)) {
							return &boss;
						}

						return nullptr;
					};

					if (Object* target = find_target()) {
						float hsp = lengthdir_x(b->spd, b->dir);
						float vsp = lengthdir_y(b->spd, b->dir);
						float dx = target->x - b->x;
						float dy = target->y - b->y;
						dx = clamp(dx, -12.0f, 12.0f);
						dy = clamp(dy, -12.0f, 12.0f);
						hsp = approach(hsp, dx, 1.5f * delta);
						vsp = approach(vsp, dy, 1.5f * delta);
						b->spd = point_distance(0.0f, 0.0f, hsp, vsp);
						b->dir = point_direction(0.0f, 0.0f, hsp, vsp);
					} else {
						if (b->spd < 10.0f) {
							b->spd += 1.0f * delta;
						}
					}

					break;
				}
			}
		}
	}

	// Physics update
	{
		const int steps = 4;

		for (int i = 0; i < steps; i++) {
			physics_update(delta / (float)steps);
		}
	}

	// Late update
	{
		player.x = clamp(player.x, 0.0f, (float)PLAY_AREA_W);
		player.y = clamp(player.y, 0.0f, (float)PLAY_AREA_H);

		For (b, bullets) {
			b->dir = wrap(b->dir, 360.0f);

			if (out_of_bounds(b->x, b->y)) {
				object_cleanup(b);
				Remove(b, bullets);
			}
		}

		For (b, p_bullets) {
			if (out_of_bounds(b->x, b->y)) {
				object_cleanup(b);
				Remove(b, p_bullets);
			}
		}
	}

	// Call coroutines
	{
		coro_memory = 0;

		if (co) {
			if (co->state == MCO_SUSPENDED) {
				mco_resume(co);
				coro_memory += co->coro_size;
			} else if (co->state == MCO_DEAD) {
				mco_destroy(co);
				co = nullptr;
			} else {
				Assert(!"unexpected mco_coro state");
			}
		}

		if (!(boss.flags & FLAG_INSTANCE_DEAD)) {
			if (boss.co) {
				if (boss.co->state == MCO_SUSPENDED) {
					boss.co->user_data = &boss;
					mco_resume(boss.co);
					coro_memory += boss.co->coro_size;
				} else if (boss.co->state == MCO_DEAD) {
					mco_destroy(boss.co);
					boss.co = nullptr;
				} else {
					Assert(!"unexpected mco_coro state");
				}
			}
		}

		For (b, bullets) {
			if (b->co) {
				if (b->co->state == MCO_SUSPENDED) {
					b->co->user_data = b;
					mco_resume(b->co);
					coro_memory += b->co->coro_size;
				} else if (b->co->state == MCO_DEAD) {
					mco_destroy(b->co);
					b->co = nullptr;
				} else {
					Assert(!"unexpected mco_coro state");
				}
			}
		}
	}


	{
		bool spellcard_bg_on_screen = false;

		if (!(boss.flags & FLAG_INSTANCE_DEAD)) {
			if (boss.GetPhase()->type == PHASE_SPELLCARD) {
				if (boss.state != BOSS_WAITING_FOR_SPELL_TO_END) {
					spellcard_bg_on_screen = true;
				}
			}
		}

		if (spellcard_bg_on_screen) {
			boss_spellcard_background_alpha = approach(boss_spellcard_background_alpha,
													   1.0f,
													   1.0f / 30.0f * delta);
		} else {
			boss_spellcard_background_alpha = 0.0f;
		}
	}

	if (is_key_pressed(SDL_SCANCODE_H)) {
		show_hitboxes ^= true;
	}

	if (is_key_pressed(SDL_SCANCODE_B)) {
		if (!(boss.flags & FLAG_INSTANCE_DEAD)) {
			boss.timer = 0;
			boss.wait_timer = 0;
		}
	}

}

void World::draw(float delta) {

	glViewport(0, 0, GAME_W, GAME_H);
	r->proj = glm::ortho(0.0f, (float)GAME_W, (float)GAME_H, 0.0f);

	r->draw_texture(GetTexture(tex_background));

	// 
	// Draw UI.
	// 
	{
		float x = PLAY_AREA_X + PLAY_AREA_W + 16;
		float y = PLAY_AREA_Y + 32;

		r->draw_text(GetSprite(spr_font_main), "HiScore", x, y);
		y += 16;

		r->draw_text(GetSprite(spr_font_main), "Score", x, y);
		y += 16 * 2;

		r->draw_text(GetSprite(spr_font_main), "Player", x, y);
		y += 16;

		r->draw_text(GetSprite(spr_font_main), "Bomb", x, y);
		y += 16;

		r->draw_text(GetSprite(spr_font_main), "Power", x, y);
		y += 16 * 2;

		r->draw_text(GetSprite(spr_font_main), "Graze", x, y);
		y += 16;

		r->draw_text(GetSprite(spr_font_main), "Point", x, y);
		y += 16;
	}

	r->break_batch();

	glViewport(PLAY_AREA_X, PLAY_AREA_Y, PLAY_AREA_W, PLAY_AREA_H);
	r->proj = glm::ortho(0.0f, (float)PLAY_AREA_W, (float)PLAY_AREA_H, 0.0f);

	if (boss_spellcard_background_alpha < 1) {
		StageData* stage = GetStageData(stage_index);
		if (stage->draw_background) {
			stage->draw_background(delta);
		}
	}

	if (boss_spellcard_background_alpha > 0) {
		if (!(boss.flags & FLAG_INSTANCE_DEAD)) {
			BossData* data = boss.GetData();
			if (data->draw_spellcard_background) {
				data->draw_spellcard_background(delta);
			}
		}
	}

	// 
	// Draw game objects.
	// 

	if (!(boss.flags & FLAG_INSTANCE_DEAD)) {
		Boss* b = &boss;

		r->draw_sprite(b->GetSprite(), (int)b->frame_index, {b->x, b->y});
	}

	player_draw(&player, delta);

	For (b, p_bullets) {
		glm::vec4 color = {1, 1, 1, 0.35f};
		glm::vec2 scale{1.5f};
		r->draw_sprite(b->GetSprite(), (int)b->frame_index, {b->x, b->y}, scale, 0, color);
	}

	For (b, bullets) {
		float dir = 0.0f;
		if (b->flags & FLAG_BULLET_ROTATE) {
			dir = b->dir - 90.0f;
		}
		r->draw_sprite(b->GetSprite(), (int)b->frame_index, {b->x, b->y}, {1.0f, 1.0f}, dir);
	}


	if (show_hitboxes) {
		r->draw_rectangle({0, 0, PLAY_AREA_W, PLAY_AREA_H}, {0.0f, 0.0f, 0.0f, 0.5f});

		if (!(boss.flags & FLAG_INSTANCE_DEAD)) {
			r->draw_circle({boss.x, boss.y}, boss.radius, {1, 1, 1, 0.5f});
		}

		{
			r->draw_circle({player.x, player.y}, player.radius);
			r->draw_circle({player.x, player.y}, player.GetCharacter()->graze_radius, {1, 1, 1, 0.25f});
		}

		For (b, p_bullets) {
			r->draw_circle({b->x, b->y}, b->radius, {1, 1, 1, 0.5f});
		}

		For (b, bullets) {
			r->draw_circle({b->x, b->y}, b->radius);
		}
	}

	// 
	// Draw boss UI.
	// 
	if (!(boss.flags & FLAG_INSTANCE_DEAD)) {
		Boss* b = &boss;
		BossData* data = b->GetData();
		BossPhase* phase = b->GetPhase();

		// Draw healthbar
		{
			int healthbar_x = 32 + 2;
			int healthbar_y = 6;
			int healthbar_w = PLAY_AREA_W - 64 - 4;
			int healthbar_h = 2;
			int reduced_w = (int) ((float)healthbar_w * (b->hp / phase->hp));

			r->draw_rectangle({healthbar_x, healthbar_y + 1, reduced_w, healthbar_h}, color_black);
			r->draw_rectangle({healthbar_x, healthbar_y, reduced_w, healthbar_h}, color_white);
		}

		// Draw phase count
		{
			char buf[10];
			stb_snprintf(buf, sizeof(buf), "%d", data->phase_count - b->phase_index - 1);
			r->draw_text(GetSprite(spr_font_main), buf, 0, 0);
		}

		// Draw timer
		{
			char buf[10];
			stb_snprintf(buf, sizeof(buf), "%d", (int)b->timer / 60);
			r->draw_text(GetSprite(spr_font_main), buf, PLAY_AREA_W, 0, HALIGN_RIGHT);
		}

		// Draw phase name
		{
			r->draw_text(GetSprite(spr_font_main), phase->name, PLAY_AREA_W, 16, HALIGN_RIGHT);
		}
	}

	r->break_batch();

}

instance_id World::get_instance_id(ObjType type) {
	Assert(next_instance_id < (u64)UINT32_MAX);
	return (type << 32) | (next_instance_id++);
}

typedef ptrdiff_t ssize;

template <typename T>
static T* BinarySearch(Arena_Backed_Array<T> arr, instance_id id) {
	ssize left = 0;
	ssize right = (ssize)arr.count - 1;

	while (left <= right) {
		ssize middle = (left + right) / 2;
		if (arr[middle].id < id) {
			left = middle + 1;
		} else if (arr[middle].id > id) {
			right = middle - 1;
		} else {
			return &arr[middle];
		}
	}

	return nullptr;
}

Bullet* World::find_bullet(instance_id id) {
	return BinarySearch(bullets, id);
}

void LaunchTowardsPoint(Object* o, float target_x, float target_y, float acc) {
	acc = fabsf(acc);
	float dist = point_distance(o->x, o->y, target_x, target_y);
	o->spd = sqrtf(dist * acc * 2.0f);
	o->acc = -acc;
	o->dir = point_direction(o->x, o->y, target_x, target_y);
}
