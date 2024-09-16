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

#define PLAYER_STARTING_X ((float)PLAY_AREA_W / 2.0f)
#define PLAYER_STARTING_Y 384.0f

#define BOSS_STARTING_X ((float)PLAY_AREA_W / 2.0f)
#define BOSS_STARTING_Y 96.0f

#define MAX_POWER 128

#define MAX_ANIMATIONS 1'000
#define TEMP_STORAGE_FOR_BOSS Kilobytes(10)

struct World;
extern World* w;

void LaunchTowardsPoint(Object* o, float target_x, float target_y, float acc);

void drop_pickup(float x, float y, PickupType type);

void get_score(int score);
void get_lives(int lives);
void get_bombs(int bombs);
void get_power(int power);
void get_graze(int graze);
void get_points(int points);

struct World {
	Player player;
	Boss   boss;

	// 
	// @Leak
	// When array runs out of capacity and replaces the
	// last object, it doesn't get cleaned up.
	// 
	dynamic_array_cap<Enemy>        enemies;
	dynamic_array_cap<Bullet>       bullets;
	dynamic_array_cap<PlayerBullet> p_bullets;
	dynamic_array_cap<Pickup>       pickups;

	u64 next_instance_id = 1;
	mco_coro* co;
	float coro_timer;
	xoshiro128plus random{{0x68756F54, 0x7250756F, 0x63656A6F, 0x35393474}};
	bool paused;
	Menu pause_menu;

	dynamic_array_cap<Animation> animations;
	Particle_System part_sys;
	float delta_multiplier = 1;

	float boss_spellcard_background_alpha;
	float boss_pcb_youmu_effect;
	Arena temp_arena_for_boss;

	bool show_hitboxes;
	size_t coro_memory;

	struct D3D {
		vec3 cam_pos;
		float pitch;
		float yaw;

		vec3 get_camera_forward();
		vec3 get_up_vector() { return {0, 1, 0}; }

		mat4 get_view_mat();
		mat4 get_proj_mat();

		mat4 get_mvp();
	} d3d;

	void init();
	void destroy();

	void physics_update(float delta);
	void update(float delta_not_modified);
	void draw(float delta_not_modified);

	instance_id get_instance_id(ObjType type);

	Bullet* find_bullet(instance_id id);
	Player* find_player(instance_id id);
	Boss*   find_boss(instance_id id);

	Object* find_object(instance_id id);
};
