#pragma once

#include "common.h"
#include "objects.h"
#include "array.h"
#include "xoshiro128plus.h"
#include <minicoro/minicoro.h>

#define MAX_BULLETS 10'000
#define MAX_PLAYER_BULLETS 1'000

#define PLAYER_STARTING_X ((float)PLAY_AREA_W / 2.0f)
#define PLAYER_STARTING_Y 384.0f

#define BOSS_STARTING_X ((float)PLAY_AREA_W / 2.0f)
#define BOSS_STARTING_Y 96.0f

#define MAX_POWER 128

struct World;
extern World* w;

void LaunchTowardsPoint(Object* o, float target_x, float target_y, float acc);

struct World {
	Player player;
	Boss boss;
	Arena_Backed_Array<Bullet> bullets;
	Arena_Backed_Array<PlayerBullet> p_bullets;

	u32 stage_index;

	u64 next_instance_id = 1;

	mco_coro* co;

	xoshiro128plus random{{0x68756F54, 0x7250756F, 0x63656A6F, 0x35393474}};

	float boss_spellcard_background_alpha;

	bool show_hitboxes;
	size_t coro_memory;

	void init();
	void destroy();

	void physics_update(float delta);
	void update(float delta);
	void draw(float delta);

	instance_id get_instance_id(ObjType type);

	Bullet* find_bullet(instance_id id);
};
