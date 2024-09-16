#pragma once

#include "game.h"

#define self (((Coro_User_Data*)co->user_data)->self)

enum {
	spr_bullet_pellet = spr_bullet_shard,
};

static void co_check_stack_usage(mco_coro* co) {
	volatile size_t dummy;
	size_t stack_addr = (size_t)&dummy;
	size_t stack_min = (size_t)co->stack_base;
	size_t stack_max = stack_min + co->stack_size;
	// log_info("start_addr: %zu", stack_addr);
	// log_info("stack_min: %zu", stack_min);
	// log_info("stack_max: %zu", stack_max);
	// log_info("coroutine stack usage: " Size_Fmt, Size_Arg(stack_max - stack_addr));
	log_info("coroutine stack usage: %zu", (stack_max - stack_addr));
}

static void wait(float time) {
	mco_coro* co = mco_running();
	Assert(co);

	mco_result res;

#if 1
	float timer;

	res = mco_pop(co, &timer, sizeof timer);
	Assert(res == MCO_SUCCESS);

	timer += time;
	while (timer >= 1) {
		res = mco_yield(co);
		Assert(res == MCO_SUCCESS);

		timer -= 1;
	}

	res = mco_push(co, &timer, sizeof timer);
	Assert(res == MCO_SUCCESS);
#else
	// unfinished
	Coro_User_Data* user = (Coro_User_Data*) co->user_data;
	user->co->timer -= time - 1;

	res = mco_yield(co);
	Assert(res == MCO_SUCCESS);
#endif
}

static Bullet* ShootExtO(Object* o,
						 float x, float y,
						 float spd, float dir, float acc,
						 u32 sprite_index, int frame_index,
						 u32 flags = 0, void (*script)(mco_coro*) = nullptr) {
	Bullet b = {};

	object_init(&b, OBJ_TYPE_BULLET);
	b.x = x;
	b.y = y;
	b.spd = spd;
	b.dir = dir;
	b.acc = acc;
	b.sprite_index = sprite_index;
	b.frame_index = (float)frame_index;
	b.flags = flags;
	b.owner = o->id;
	b.bullet_type = BULLET_TYPE_BULLET;

	switch (sprite_index) {
		case spr_bullet_arrow:	 b.radius = 2.5f; b.flags |= FLAG_BULLET_ROTATE; break;
		case spr_bullet_outline: b.radius = 2.5f; break;
		case spr_bullet_filled:	 b.radius = 3.5f; break;
		case spr_bullet_rice:	 b.radius = 2.5f; b.flags |= FLAG_BULLET_ROTATE; break;
		case spr_bullet_kunai:	 b.radius = 2.5f; b.flags |= FLAG_BULLET_ROTATE; break;
		case spr_bullet_shard:	 b.radius = 2.5f; b.flags |= FLAG_BULLET_ROTATE; break;
		case spr_bullet_card:	 b.radius = 2.5f; b.flags |= FLAG_BULLET_ROTATE; break;
		case spr_bullet_bullet:	 b.radius = 2.5f; b.flags |= FLAG_BULLET_ROTATE; break;

		case spr_bullet_small:	 b.radius = 2.5f; break;
	}

	if (script) {
		coroutine_create(&b.co, script);
	}

	play_sound(snd_enemy_shoot);

	return array_add(&w->bullets, b);
}

static Bullet* ShootO(Object* o,
					  float spd, float dir, float acc,
					  u32 sprite_index, int frame_index,
					  u32 flags = 0, void (*script)(mco_coro*) = nullptr) {
	return ShootExtO(o, o->x, o->y, spd, dir, acc, sprite_index, frame_index, flags, script);
}

static Bullet* ShootLazerO(Object* o,
						   float spd, float dir,
						   float target_length, float thickness,
						   int frame_index) {
	// Don't divide by zero.
	Assert(spd != 0);
	Assert(target_length != 0);
	
	Bullet b = {};

	object_init(&b, OBJ_TYPE_BULLET);
	b.x = o->x;
	b.y = o->y;
	b.spd = spd;
	b.dir = dir;
	b.sprite_index = spr_lazer;
	b.frame_index = (float)frame_index;

	b.bullet_type = BULLET_TYPE_LAZER;
	b.lazer.target_length = target_length;
	b.lazer.thickness = thickness;
	b.lazer.time = target_length / spd;

	play_sound(snd_lazer);

	return array_add(&w->bullets, b);
}



static Bullet* ShootExt(float x, float y,
						float spd, float dir, float acc,
						u32 sprite_index, int frame_index,
						u32 flags = 0, void (*script)(mco_coro*) = nullptr) {
	mco_coro* co = mco_running();
	return ShootExtO(self, x, y, spd, dir, acc, sprite_index, frame_index, flags, script);
}

static Bullet* Shoot(float spd, float dir, float acc,
					 u32 sprite_index, int frame_index,
					 u32 flags = 0, void (*script)(mco_coro*) = nullptr) {
	mco_coro* co = mco_running();
	return ShootO(self, spd, dir, acc, sprite_index, frame_index, flags, script);
}

static Bullet* ShootLazer(float spd, float dir,
						  float target_length, float thickness,
						  int frame_index) {
	mco_coro* co = mco_running();
	return ShootLazerO(self, spd, dir, target_length, thickness, frame_index);
}



template <typename Func>
static void ShootRadial(int count, float dir_diff, const Func& func) {
	for (int i = 0; i < count; i++) {
		float mul = -(count - 1) / 2.0f + i;

		auto res = func();

		if constexpr (std::is_same_v<decltype(res), Bullet*>) {
			Bullet* b = res;

			b->dir += dir_diff * mul;

		} else if constexpr (std::is_same_v<decltype(res), array<instance_id>>) {
			For (it, res) {
				Bullet* b = w->find_bullet(*it);
				Assert(b);

				b->dir += dir_diff * mul;
			}

		} else {
			Assert(false);
		}
	}
}

// 
// "A" stands for alloc.
// 
// Allocates from temporary arena (w->temp_arena_for_boss). You have to
// clear it once in a while in your script if you use functions that allocate.
// 
template <typename Func>
static array<instance_id> ShootRadialA(int count, float dir_diff, const Func& func) {

	// 
	// @Hack?
	// I use arena_push here to get an aligned pointer
	// 
	array<instance_id> bullets;
	bullets.data  = (instance_id*) arena_push(&w->temp_arena_for_boss, 0);
	bullets.count = 0;

	for (int i = 0; i < count; i++) {
		float mul = -(count - 1) / 2.0f + i;

		auto res = func();

		if constexpr (std::is_same_v<decltype(res), Bullet*>) {
			Bullet* b = res;

			b->dir += dir_diff * mul;

			// Make sure there is no alignment.
			arena_push(&w->temp_arena_for_boss, sizeof(instance_id), 1);
			bullets.data[bullets.count++] = b->id;

		} else if constexpr (std::is_same_v<decltype(res), array<instance_id>>) {
			For (it, res) {
				Bullet* b = w->find_bullet(*it);
				Assert(b);

				b->dir += dir_diff * mul;

				// Make sure there is no alignment.
				arena_push(&w->temp_arena_for_boss, sizeof(instance_id), 1);
				bullets.data[bullets.count++] = b->id;
			}

		} else {
			Assert(false);
		}
	}

	return bullets;
}

static float DirToPlayer(Object* o) {
	return point_direction(o->x, o->y, w->player.x, w->player.y);
}

static void Wander(Object* o) {
	float target_x = w->random.rangef(32.0f, (float)PLAY_AREA_W - 32.0f);
	float target_y = w->random.rangef(32.0f, (float)BOSS_STARTING_Y * 2.0f - 32.0f);
	float x = o->x;
	float y = o->y;
	target_x = clamp(target_x, x - 80.0f, x + 80.0f);
	target_y = clamp(target_y, y - 80.0f, y + 80.0f);
	LaunchTowardsPoint(o, target_x, target_y, 0.01f);
}

static void GoBack(Object* o) {
	LaunchTowardsPoint(o, PLAY_AREA_W / 2, 64, 0.02f);
}

static Boss* CreateBoss(u32 boss_index,
						float x = BOSS_STARTING_X, float y = BOSS_STARTING_Y) {
	Boss* b = &w->boss;

	// @Cleanup: Why is this commented out?
	Assert(b->flags & FLAG_INSTANCE_DEAD);
	// if (!(b->flags & FLAG_INSTANCE_DEAD)) {
	// 	object_cleanup(b);
	// }

	*b = {};
	object_init(b, OBJ_TYPE_BOSS);
	b->x = x;
	b->y = y;
	b->radius = 25;
	b->boss_index = boss_index;

	BossData* data = b->GetData();
	b->sprite_index = data->spr_idle;

	boss_start_phase(b);

	return b;
}

static Enemy* CreateEnemy(float x, float y, float spd, float dir, float acc,
						  u32 sprite_index, float hp, int drops,
						  void (*script)(mco_coro*) = nullptr,
						  void (*death_callback)(Object*) = nullptr,
						  void (*update_callback)(Object*, float) = nullptr) {
	Enemy e = {};

	object_init(&e, OBJ_TYPE_ENEMY);
	e.x = x;
	e.y = y;
	e.spd = spd;
	e.dir = dir;
	e.acc = acc;
	e.sprite_index = sprite_index;
	e.hp = hp;
	e.drops = drops;
	e.radius = 10;

	if (script) {
		coroutine_create(&e.co, script);
	}

	e.death_callback = death_callback;
	e.update_callback = update_callback;

	return array_add(&w->enemies, e);
}

static float seconds(float sec) {
	return sec * 60.0f;
}

static void Kira(Object* o) {
	boss_create_kira_particle(o);
	play_sound(snd_kira);
}
