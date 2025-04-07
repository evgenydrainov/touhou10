#pragma once

#include "common.h"

#include "objects.h"
#include "particle_system.h"
#include "menu.h"

#include "xoshiro128plus.h"
#include <minicoro/minicoro.h>

#define MAX_BULLETS 10'000
#define MAX_PLAYER_BULLETS 1'000
#define MAX_ENEMIES 1'000
#define MAX_PICKUPS 1'000

#define PLAY_AREA_W 384
#define PLAY_AREA_H 448

#define PLAYER_STARTING_X ((float)PLAY_AREA_W / 2.0f)
#define PLAYER_STARTING_Y 384.0f

#define BOSS_STARTING_X ((float)PLAY_AREA_W / 2.0f)
#define BOSS_STARTING_Y 96.0f

#define MAX_POWER 128

#define MAX_ANIMATIONS 1'000
#define TEMP_STORAGE_FOR_BOSS Kilobytes(10)

void LaunchTowardsPoint(Object* o, float target_x, float target_y, float acc);

void drop_pickup(float x, float y, PickupType type);

void get_score(int score);
void get_lives(int lives);
void get_bombs(int bombs);
void get_power(int power);
void get_graze(int graze);
void get_points(int points);

struct Coro_User_Data {
	Coroutine* co;
	Object* self;
};

struct World {
	Player player;
	Boss   boss;

	bump_array<Enemy>        enemies;
	bump_array<Bullet>       bullets;
	bump_array<PlayerBullet> p_bullets;
	bump_array<Pickup>       pickups;

	u64 next_instance_id = 1;
	Coroutine co;
	xoshiro128plus rng{{0x68756F54, 0x7250756F, 0x63656A6F, 0x35393474}};
	bool paused;
	Menu pause_menu;

	bump_array<Animation> animations;
	float delta_multiplier = 1;

	float boss_spellcard_background_alpha;
	float boss_pcb_youmu_effect;
	Arena temp_arena_for_boss;

	bool show_hitboxes;
	size_t coro_memory;

	struct Cam3D {
		vec3 pos;
		float pitch;
		float yaw;
	} cam3d;

	struct DeathEffect {
		bool show;
		float t;
		float x;
		float y;
	} death_effect;

	void init();
	void deinit();

	void physics_update(float delta, float delta_not_modified);
	void update(float delta_not_modified);
	void draw(float delta_not_modified);

	instance_id get_instance_id(ObjType type);

	Bullet* find_bullet(instance_id id);
	Player* find_player(instance_id id);
	Boss*   find_boss(instance_id id);

	Object* find_object(instance_id id);

	vec3 cam3d_get_camera_forward();
	vec3 cam3d_get_up_vector();

	mat4 cam3d_get_view_mat();
	mat4 cam3d_get_proj_mat();

	mat4 cam3d_get_mvp();
};

extern World world;
