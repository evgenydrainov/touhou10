#include "world.h"

#include "game.h"
#include "cpml.h"

#include <glad/gl.h>
#include <glm/gtc/matrix_transform.hpp>

World* w;

static const Particle_Type part_type_graze = {
	/* speed_min      = */ 4,
	/* speed_max      = */ 6,
	/* speed_incr     = */ -0.25f,
	/* direction_min  = */ 0,
	/* direction_max  = */ 360,
	/* direction_incr = */ 0,
	/* lifespan_min   = */ 10,
	/* lifespan_max   = */ 15,
	/* sprite_index   = */ spr_particle_graze,
	/* color          = */ {1, 1, 1, 0.5f},
};

void World::init() {
	player_init(&player);

	boss.flags |= FLAG_INSTANCE_DEAD;

	enemies    = ArrayAllocFromArena<Enemy>        (&g->arena, MAX_ENEMIES);
	bullets    = ArrayAllocFromArena<Bullet>       (&g->arena, MAX_BULLETS);
	p_bullets  = ArrayAllocFromArena<PlayerBullet> (&g->arena, MAX_PLAYER_BULLETS);
	pickups    = ArrayAllocFromArena<Pickup>       (&g->arena, MAX_PICKUPS);
	animations = ArrayAllocFromArena<Animation>    (&g->arena, MAX_ANIMATIONS);

	part_sys.init();

	{
		mco_desc desc = mco_desc_init(GetStageData(g->stage_index)->script, 0);
		mco_create(&co, &desc);
	}

	temp_arena_for_boss = ArenaAllocFromArena(&g->arena, TEMP_STORAGE_FOR_BOSS);
}

void World::destroy() {

	if (co) {
		mco_destroy(co);
		co = nullptr;
	}

	part_sys.destroy();

	For (p, pickups) {
		object_cleanup(p);
	}
	pickups.clear();

	For (b, p_bullets) {
		object_cleanup(b);
	}
	p_bullets.clear();

	For (b, bullets) {
		object_cleanup(b);
	}
	bullets.clear();

	For (e, enemies) {
		object_cleanup(e);
	}
	enemies.clear();

	if (!(boss.flags & FLAG_INSTANCE_DEAD)) {
		object_cleanup(&boss);
	}
	boss.flags |= FLAG_INSTANCE_DEAD;

	object_cleanup(&player);
}

static bool player_collides_with_bullet(Player* p, float player_radius, Bullet* b) {
	bool result = false;

	switch (b->bullet_type) {
		case BULLET_TYPE_BULLET: {
			result = circle_vs_circle(p->x, p->y, player_radius, b->x, b->y, b->radius);
			break;
		}

		case BULLET_TYPE_LAZER: {
			float rect_center_x = b->x + lengthdir_x(b->lazer.length / 2.0f, b->dir);
			float rect_center_y = b->y + lengthdir_y(b->lazer.length / 2.0f, b->dir);
			result = circle_vs_rotated_rect(p->x, p->y, player_radius, rect_center_x, rect_center_y, b->lazer.thickness, b->lazer.length, b->dir);
			break;
		}
	}

	return result;
}

void World::physics_update(float delta) {

	auto object_move = [](Object* o, float delta) {
		o->x += o->spd * cosf(glm::radians(o->dir)) * delta;
		o->y -= o->spd * sinf(glm::radians(o->dir)) * delta;
		o->spd += o->acc * delta;
		o->spd = max(o->spd, 0.0f);
	};

	player.x += player.hsp * delta;
	player.y += player.vsp * delta;

	object_move(&boss, delta);

	For (e, enemies) {
		object_move(e, delta);
	}

	For (b, bullets) {
		switch (b->bullet_type) {
			case BULLET_TYPE_LAZER:
				// Fallthrough
				if (b->lazer.timer < b->lazer.time) {
					break;
				}
			case BULLET_TYPE_BULLET: {
				object_move(b, delta);
				break;
			}
		}
	}

	For (b, p_bullets) {
		object_move(b, delta);
	}

	For (p, pickups) {
		p->x += p->hsp * delta;
		p->y += p->vsp * delta;
	}

	// 
	// Check collisions.
	// 

	// Player vs bullets
	For (b, bullets) {
		if (player_collides_with_bullet(&player, player.GetCharacter()->graze_radius, b)) {
			if (player.state == PLAYER_STATE_NORMAL) {

				bool can_graze;
				if (b->bullet_type == BULLET_TYPE_LAZER) {
					can_graze = player.lazer_graze_timer <= 0;
					player.lazer_graze_timer -= delta;
				} else {
					can_graze = !(b->flags & FLAG_BULLET_WAS_GRAZED);
				}

				if (can_graze) {
					get_graze(1);
					part_sys.create_particle({player.x, player.y}, part_type_graze);
					b->flags |= FLAG_BULLET_WAS_GRAZED;

					if (b->bullet_type == BULLET_TYPE_LAZER) {
						player.lazer_graze_timer += 8;
					}
				}
			}
		}

		if (player_collides_with_bullet(&player, player.radius, b)) {
			if (player.state == PLAYER_STATE_NORMAL) {
				if (player.iframes <= 0) {
					player.state = PLAYER_STATE_DYING;
					player.timer = PLAYER_DEATH_TIME;

					play_sound(snd_pichuun);
				}

				object_cleanup(b);
				Remove(b, bullets);
			}
		}
	}

	// Player vs enemies
	For (e, enemies) {
		if (circle_vs_circle(player.x, player.y, player.radius, e->x, e->y, e->radius)) {
			if (player.state == PLAYER_STATE_NORMAL) {
				if (player.iframes <= 0) {
					player.state = PLAYER_STATE_DYING;
					player.timer = PLAYER_DEATH_TIME;

					play_sound(snd_pichuun);
				}
			}
		}
	}

	// Player vs boss
	if (!(boss.flags & FLAG_INSTANCE_DEAD)) {
		if (circle_vs_circle(player.x, player.y, player.radius, boss.x, boss.y, boss.radius)) {
			if (player.state == PLAYER_STATE_NORMAL) {
				if (player.iframes <= 0) {
					player.state = PLAYER_STATE_DYING;
					player.timer = PLAYER_DEATH_TIME;

					play_sound(snd_pichuun);
				}
			}
		}
	}

	// Boss vs player bullets
	if (!(boss.flags & FLAG_INSTANCE_DEAD)) {
		For (b, p_bullets) {
			if (circle_vs_circle(boss.x, boss.y, boss.radius, b->x, b->y, b->radius)) {
				if (boss.state == BOSS_STATE_NORMAL) {
					boss.hp -= b->dmg;
				}

				play_sound(snd_enemy_hurt);

				object_cleanup(b);
				Remove(b, p_bullets);
			}
		}
	}

	// Enemy vs player bullets
	For (e, enemies) {
		For (b, p_bullets) {
			if (circle_vs_circle(e->x, e->y, e->radius, b->x, b->y, b->radius)) {
				e->hp -= b->dmg;

				play_sound(snd_enemy_hurt);

				object_cleanup(b);
				Remove(b, p_bullets);
			}
		}
	}

	// Player vs pickups
	For (p, pickups) {
		if (circle_vs_circle(player.x, player.y, player.GetCharacter()->graze_radius, p->x, p->y, p->radius)) {
			if (player.state == PLAYER_STATE_NORMAL) {
				switch (p->pickup_type) {
					case PICKUP_TYPE_POWER:
						get_power(1);
						get_score(10);
						break;
					case PICKUP_TYPE_POINT:      get_points(1);        break;
					case PICKUP_TYPE_POWER_BIG:  get_power(8);         break;
					case PICKUP_TYPE_POINT_BIG:  get_points(8);        break;
					case PICKUP_TYPE_BOMB:       get_bombs(1);         break;
					case PICKUP_TYPE_LIFE:       get_lives(1);         break;
					case PICKUP_TYPE_SCORE:      get_score(10);        break;
					case PICKUP_TYPE_FULL_POWER: get_power(MAX_POWER); break;
				}

				play_sound(snd_pickup);

				object_cleanup(p);
				Remove(p, pickups);
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

// Ignore the upper bound for pickups
static bool out_of_bounds_ignore_upper(float x, float y, float off = 50.0f) {
	if (x < -off || x > PLAY_AREA_W + off || y > PLAY_AREA_H + off) {
		return true;
	}
	return false;
}

#define PICKUP_STARTING_VSP -1.6f

void drop_pickup(float x, float y, PickupType type) {
	Pickup p = {};

	object_init(&p, OBJ_TYPE_PICKUP);
	p.x = x;
	p.y = y;
	p.vsp = PICKUP_STARTING_VSP;
	p.radius = 8;
	p.sprite_index = spr_pickup;

	p.pickup_type = type;
	p.frame_index = (float)type;

	w->pickups.add(p);
}

static void enemy_drop_pickups(int drops, float x, float y) {
	// 
	// All possible enemy drops
	// 

	switch (drops) {
		case 2:
			// A power or a point at a 50% chance
			if (!(w->random.rangef(0.0f, 1.0f) < 0.5f)) {
				break;
			}
			// Fallthrough
		case 1: {
			// A power or a point
			PickupType types[] = {PICKUP_TYPE_POWER, PICKUP_TYPE_POINT};
			drop_pickup(x, y, w->random.index(types));
			break;
		}
	}
}

void World::update(float delta_not_modified) {

	float delta = delta_not_modified * delta_multiplier;

	if (!(SDL_GetWindowFlags(g->window) & SDL_WINDOW_INPUT_FOCUS)
		|| (SDL_GetWindowFlags(g->window) & SDL_WINDOW_MINIMIZED)) {
		paused = true;
	}

	if (paused) {
		goto l_skip_update;
	}

	// Update
	{
		player_update(&player, delta);

		if (!(boss.flags & FLAG_INSTANCE_DEAD)) {
			boss_update(&boss, delta);
		}

		size_t enemy_count = enemies.count;

		For (e, enemies) {
			// Make sure noone messes with the array during this loop
			Assert(enemies.count == enemy_count);

			if (out_of_bounds(e->x, e->y)) {
				// Don't call the death callback and don't drop pickups when enemy leaves the screen
				object_cleanup(e);
				Remove(e, enemies);
				enemy_count--;
				continue;
			}

			if (e->hp <= 0) {
				enemy_drop_pickups(e->drops, e->x, e->y);

				if (e->death_callback) {
					e->death_callback(e);
				}

				play_sound(snd_enemy_die);

				object_cleanup(e);
				Remove(e, enemies);
				enemy_count--;
				continue;
			}

			e->dir = wrapf(e->dir, 360.0f);

			if (e->update_callback) {
				e->update_callback(e, delta);
			}

			e->frame_index = object_animate(e->sprite_index, e->frame_index, delta);
		}

		For (b, bullets) {
			if (out_of_bounds(b->x, b->y)) {
				object_cleanup(b);
				Remove(b, bullets);
				continue;
			}

			b->dir = wrapf(b->dir, 360.0f);

			switch (b->bullet_type) {
				case BULLET_TYPE_LAZER: {
					if (b->lazer.timer < b->lazer.time) {
						b->lazer.timer += delta;
						b->lazer.length = lerp(0.0f, b->lazer.target_length, b->lazer.timer / b->lazer.time);
					} else {
						b->lazer.length = b->lazer.target_length;
					}
					break;
				}
			}
		}

		For (b, p_bullets) {
			if (out_of_bounds(b->x, b->y)) {
				object_cleanup(b);
				Remove(b, p_bullets);
				continue;
			}

			switch (b->type) {
				case PLAYER_BULLET_REIMU_CARD: {
					b->reimu_card.rotation += 16 * delta;
					break;
				}

				case PLAYER_BULLET_REIMU_ORB_SHOT: {

					auto find_target = [&]() -> Object* {
						Object* result = nullptr;
						float dist_closest = INFINITY;

						if (!(boss.flags & FLAG_INSTANCE_DEAD)) {
							result = &boss;
							dist_closest = point_distance(b->x, b->y, boss.x, boss.y);
						}

						For (e, enemies) {
							float dist = point_distance(b->x, b->y, e->x, e->y);
							if (dist < dist_closest) {
								result = e;
								dist_closest = dist;
							}
						}

						return result;
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

		For (p, pickups) {
			if (out_of_bounds_ignore_upper(p->x, p->y)) {
				object_cleanup(p);
				Remove(p, pickups);
				continue;
			}

			if (p->homing_target != 0) {
				bool stop_homing = true;

				Object* target = find_object(p->homing_target);
				if (target) {
					stop_homing = false;

					if (target->GetType() == OBJ_TYPE_PLAYER) {
						if (((Player*)target)->state != PLAYER_STATE_NORMAL) {
							stop_homing = true;
						}
					}
				}

				if (stop_homing) {
					p->hsp = 0;
					p->vsp = PICKUP_STARTING_VSP;
					p->homing_target = 0;
				} else {
					float spd = 8;
					float dir = point_direction(p->x, p->y, player.x, player.y);
					p->hsp = lengthdir_x(spd, dir);
					p->vsp = lengthdir_y(spd, dir);
				}
			} else {
				const float gravity = 0.03f;

				p->vsp += gravity * delta;
				p->vsp = min(p->vsp, 3.0f);
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

		// 
		// Does it matter that we destroy out of bounds objects earlier
		// or sould we do it here?
		// 'dir' wrap also.
		// 
	}

	// Call coroutines
	{
		auto handle_coroutine = [&](mco_coro* &co, Object* self) {
			if (co) {
				if (co->state == MCO_SUSPENDED) {
					co->user_data = self;
					mco_resume(co);
					coro_memory += co->coro_size;
				} else if (co->state == MCO_DEAD) {
					mco_destroy(co);
					co = nullptr;
				} else {
					Assert(!"unexpected mco_coro state");
				}
			}
		};

		coro_memory = 0;

		handle_coroutine(co, nullptr);

		if (!(boss.flags & FLAG_INSTANCE_DEAD)) {
			handle_coroutine(boss.co, &boss);
		}

		For (e, enemies) {
			handle_coroutine(e->co, e);
		}

		For (b, bullets) {
			handle_coroutine(b->co, b);
		}
	}

	For (a, animations) {
		a->time += a->speed * delta / 60.0f;
		if (a->time >= a->data->length) {
			Remove(a, animations);
		}
	}

	part_sys.update(delta);


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

l_skip_update:

	if (is_key_pressed(SDL_SCANCODE_ESCAPE)) {
		paused ^= true;
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

void World::draw(float delta_not_modified) {

	float delta = delta_not_modified * delta_multiplier;

	glViewport(0, 0, GAME_W, GAME_H);
	r->proj = glm::ortho(0.0f, (float)GAME_W, (float)GAME_H, 0.0f);

	// Draw background
	{
		Rect src = {0, 0, GAME_W, GAME_H};
		r->draw_texture(GetTexture(tex_background), src);

		r->break_batch();
	}

	// 
	// Draw UI.
	// 
	{
		float x = PLAY_AREA_X + PLAY_AREA_W + 16;
		float y = PLAY_AREA_Y + 32;

		Static_String<64> buf;

		Sprintf(&buf, "HiScore %d", 0);
		r->draw_text(GetSprite(spr_font_main), buf, x, y);
		y += 16;

		Sprintf(&buf, "Score %d", g->stats.score);
		r->draw_text(GetSprite(spr_font_main), buf, x, y);
		y += 16 * 2;

		Sprintf(&buf, "Player %d", g->stats.lives);
		r->draw_text(GetSprite(spr_font_main), buf, x, y);
		y += 16;

		Sprintf(&buf, "Bomb %d", g->stats.bombs);
		r->draw_text(GetSprite(spr_font_main), buf, x, y);
		y += 16 * 2;

		Sprintf(&buf, "Power %d", g->stats.power);
		r->draw_text(GetSprite(spr_font_main), buf, x, y);
		y += 16;

		Sprintf(&buf, "Graze %d", g->stats.graze);
		r->draw_text(GetSprite(spr_font_main), buf, x, y);
		y += 16;

		Sprintf(&buf, "Point %d", g->stats.points);
		r->draw_text(GetSprite(spr_font_main), buf, x, y);
		y += 16;
	}

	// Draw enemy label
	if (!(boss.flags & FLAG_INSTANCE_DEAD)) {
		float x = PLAY_AREA_X + boss.x;
		float y = PLAY_AREA_Y + PLAY_AREA_H;

		glm::vec4 color = {1, 1, 1, 0.5f};

		r->draw_sprite(GetSprite(spr_enemy_label), 0, {x, y}, {1, 1}, 0, color);
	}

	r->break_batch();

	glViewport(PLAY_AREA_X, PLAY_AREA_Y, PLAY_AREA_W, PLAY_AREA_H);
	r->proj = glm::ortho(0.0f, (float)PLAY_AREA_W, (float)PLAY_AREA_H, 0.0f);

	glScissor(PLAY_AREA_X, PLAY_AREA_Y, PLAY_AREA_W, PLAY_AREA_H);
	glEnable(GL_SCISSOR_TEST);
	Defer { glDisable(GL_SCISSOR_TEST); };

	if (boss_spellcard_background_alpha < 1) {
		StageData* stage = GetStageData(g->stage_index);
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

		float x = b->x;
		float y = b->y;

		if (b->spd <= BOSS_MOVE_THRESHOLD_VISUAL) {
			y += 2 * sinf(SDL_GetTicks() / 200.0f);
		}

		r->draw_sprite(b->GetSprite(), (int)b->frame_index, {x, y});
	}

	For (e, enemies) {
		r->draw_sprite(e->GetSprite(), (int)e->frame_index, {e->x, e->y}, {1, 1}, e->angle);
	}

	player_draw(&player, delta);

	For (b, p_bullets) {
		glm::vec4 color = {1, 1, 1, 0.35f};
		glm::vec2 scale = {1.5f, 1.5f};
		float angle = 0;
		if (b->type == PLAYER_BULLET_REIMU_CARD) {
			angle = b->reimu_card.rotation;
		}
		r->draw_sprite(b->GetSprite(), (int)b->frame_index, {b->x, b->y}, scale, angle, color);
	}

	For (b, bullets) {
		switch (b->bullet_type) {
			case BULLET_TYPE_BULLET: {
				float angle = 0.0f;
				if (b->flags & FLAG_BULLET_ROTATE) {
					angle = b->dir - 90.0f;
				}
				r->draw_sprite(b->GetSprite(), (int)b->frame_index, {b->x, b->y}, {1.0f, 1.0f}, angle);
				break;
			}

			case BULLET_TYPE_LAZER: {
				float angle = b->dir + 90.0f;
				float xscale = (b->lazer.thickness + 2.0f) / 16.0f;
				float yscale = b->lazer.length / 16.0f;
				r->draw_sprite(b->GetSprite(), (int)b->frame_index,
							   {b->x, b->y}, {xscale, yscale}, angle);
				break;
			}
		}
	}

	For (p, pickups) {
		r->draw_sprite(p->GetSprite(), (int)p->frame_index, {p->x, p->y});

		if (p->y < 0) {
			glm::vec4 color = {1, 1, 1, 0.5f};

			r->draw_sprite(p->GetSprite(), (int)p->frame_index + PICKUP_TYPE_COUNT, {p->x, 8}, {1, 1}, 0, color);
		}
	}

	part_sys.draw(delta);

	For (a, animations) {
		a->draw(delta);
	}


	if (show_hitboxes) {
		r->draw_rectangle({0, 0, PLAY_AREA_W, PLAY_AREA_H}, {0.0f, 0.0f, 0.0f, 0.5f});

		if (!(boss.flags & FLAG_INSTANCE_DEAD)) {
			r->draw_circle({boss.x, boss.y}, boss.radius, {1, 1, 1, 0.5f});
		}

		For (e, enemies) {
			r->draw_circle({e->x, e->y}, e->radius, {1, 1, 1, 0.25f});
		}

		{
			r->draw_circle({player.x, player.y}, player.radius);
			r->draw_circle({player.x, player.y}, player.GetCharacter()->graze_radius, {1, 1, 1, 0.25f});
		}

		For (b, p_bullets) {
			r->draw_circle({b->x, b->y}, b->radius, {1, 1, 1, 0.25f});
		}

		For (b, bullets) {
			switch (b->bullet_type) {
				case BULLET_TYPE_BULLET: {
					r->draw_circle({b->x, b->y}, b->radius);
					break;
				}

				case BULLET_TYPE_LAZER: {
					float angle = b->dir + 90.0f;
					float xscale = b->lazer.thickness / 16.0f;
					float yscale = b->lazer.length / 16.0f;
					r->draw_rectangle_ext({b->x, b->y}, {xscale, yscale}, {8.0f, 0.0f}, angle);
					break;
				}
			}
		}

		For (p, pickups) {
			r->draw_circle({p->x, p->y}, p->radius, {1, 1, 1, 0.25f});
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
			Static_String<10> buf;
			Sprintf(&buf, "%d", data->phase_count - b->phase_index - 1);
			r->draw_text(GetSprite(spr_font_main), buf, 0, 0);
		}

		// Draw timer
		{
			Static_String<10> buf;
			Sprintf(&buf, "%d", (int)b->timer / 60);
			r->draw_text(GetSprite(spr_font_main), buf, PLAY_AREA_W, 0, HALIGN_RIGHT);
		}

		// Draw phase name
		{
			r->draw_text(GetSprite(spr_font_main), phase->name, PLAY_AREA_W, 16, HALIGN_RIGHT);
		}
	}

	// 
	// Draw pause menu
	// 
	if (paused) {
		r->draw_rectangle({0, 0, PLAY_AREA_W, PLAY_AREA_H}, {0, 0, 0, 0.5f});

		r->draw_text(GetSprite(spr_font_main), "Paused", PLAY_AREA_W / 2.0f, PLAY_AREA_H / 2.0f, HALIGN_CENTER, VALIGN_MIDDLE);
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

Player* World::find_player(instance_id id) {
	Player* result = nullptr;
	if (player.id == id) {
		result = &player;
	}
	return result;
}

Boss* World::find_boss(instance_id id) {
	Boss* result = nullptr;
	if (!(boss.flags & FLAG_INSTANCE_DEAD)) {
		if (boss.id == id) {
			result = &boss;
		}
	}
	return result;
}

// @Todo
Object* World::find_object(instance_id id) {
	Object* result = nullptr;
	u64 object_type = id >> 32;
	switch (object_type) {
		case OBJ_TYPE_PLAYER :
			result = find_player(id);
			break;
		case OBJ_TYPE_BOSS:
			break;
		case OBJ_TYPE_ENEMY:
			break;
		case OBJ_TYPE_BULLET:
			result = find_bullet(id);
			break;
		case OBJ_TYPE_PLAYER_BULLET:
			break;
		case OBJ_TYPE_PICKUP:
			break;
	}
	return result;
}

void LaunchTowardsPoint(Object* o, float target_x, float target_y, float acc) {
	acc = fabsf(acc);
	float dist = point_distance(o->x, o->y, target_x, target_y);
	o->spd = sqrtf(dist * acc * 2.0f);
	o->acc = -acc;
	o->dir = point_direction(o->x, o->y, target_x, target_y);
}

void get_score(int score) {
	g->stats.score += score;
}

void get_lives(int lives) {
	while (lives--) {
		if (g->stats.lives < 8) {
			g->stats.lives++;
			play_sound(snd_extend);
		} else {
			get_bombs(1);
		}
	}
}

void get_bombs(int bombs) {
	while (bombs--) {
		if (g->stats.bombs < 8) {
			g->stats.bombs++;
		}
	}
}

void get_power(int power) {
	while (power--) {
		if (g->stats.power < 128) {
			g->stats.power++;
			switch (g->stats.power) {
				case 8:
				case 16:
				case 32:
				case 48:
				case 64:
				case 80:
				case 96:
				case 128: play_sound(snd_powerup); break;
			}
		}
	}
}

void get_graze(int graze) {
	g->stats.graze += graze;
	play_sound(snd_graze);
}

void get_points(int points) {
	while (points--) {
		g->stats.points++;
		if (g->stats.points >= 800) {
			if (g->stats.points % 200 == 0) {
				get_lives(1);
			}
		} else {
			switch (g->stats.points) {
				case 50:
				case 125:
				case 200:
				case 300:
				case 450: get_lives(1); break;
			}
		}
	}
}
