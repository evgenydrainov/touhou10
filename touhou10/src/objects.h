#pragma once

#include "common.h"
#include "assets.h"
#include <minicoro/minicoro.h>

// 
// High 32 bits: ObjType, low 32 bits: id
// 
typedef u64 instance_id;

enum ObjType : u64 {
	OBJ_TYPE_PLAYER = 1,
	OBJ_TYPE_BOSS,
	OBJ_TYPE_ENEMY,
	OBJ_TYPE_BULLET,
	OBJ_TYPE_PLAYER_BULLET,
	OBJ_TYPE_PICKUP,
};

enum {
	FLAG_INSTANCE_DEAD = 1,

	FLAG_BULLET_ROTATE     = 1 << 16,
	FLAG_BULLET_WAS_GRAZED = 1 << 17,

	FLAG_BOSS_WAS_HIT_THIS_FRAME = 1 << 16,
};

struct Object {
	instance_id id;
	u32 flags;

	float x;
	float y;
	float spd;
	float dir;
	float acc;
	float radius;

	u32 sprite_index;
	float frame_index;

	const Sprite& GetSprite() {
		return ::get_sprite(sprite_index);
	}

	ObjType GetType() {
		return (ObjType)(id >> 32);
	}
};

void object_init(Object* o, ObjType type);
float object_animate(u32 sprite_index, float frame_index, float delta);

#define PLAYER_STATE_ENUM(X) \
	X(PLAYER_STATE_NORMAL) \
	X(PLAYER_STATE_DYING) \
	X(PLAYER_STATE_APPEARING)

DEFINE_NAMED_ENUM(PlayerState, PLAYER_STATE_ENUM)

#define PLAYER_DEATH_TIME      15.0f
#define PLAYER_APPEAR_TIME     30.0f
#define PLAYER_RESPAWN_IFRAMES 120.0f
#define PLAYER_BOMB_TIME       (2.5f * 60.0f)

struct Player : Object {
	float hsp;
	float vsp;
	PlayerState state;
	bool focused;
	float iframes;
	float timer;
	float bomb_timer;
	float lazer_graze_timer;

	u32 character_index;
	float hitbox_alpha;

	union {
		struct {
			float fire_timer;
			int fire_queue;
		} reimu;
	};

	Character* GetCharacter() {
		return ::GetCharacter(character_index);
	}
};

void player_init(Player* p);
void player_update(Player* p, float delta);
bool player_get_hit(Player* p);
void player_draw(Player* p, float delta);

struct Coroutine {
	mco_coro* handle;
	float timer;
};

void coroutine_create(Coroutine* co, void (*func)(mco_coro*));
void coroutine_destroy(Coroutine* co);

#define BOSS_STATE_ENUM(X) \
	X(BOSS_STATE_NORMAL) \
	X(BOSS_WAITING_FOR_PHASE_TO_START) \
	X(BOSS_WAITING_FOR_SPELL_TO_END)

DEFINE_NAMED_ENUM(BossState, BOSS_STATE_ENUM)

#define BOSS_MOVE_THRESHOLD_VISUAL 0.01f

struct Boss : Object {
	u32 boss_index;
	BossState state;
	int phase_index;
	float hp;
	float timer;
	float wait_timer;

	Coroutine co;

	BossData* GetData() {
		return GetBossData(boss_index);
	}

	BossPhase* GetPhase() {
		BossData* data = GetData();
		Assert(phase_index >= 0);
		Assert(phase_index < data->phase_count);
		return &data->phases[phase_index];
	}
};

void boss_update(Boss* b, float delta);
void boss_start_phase(Boss* b);
void boss_end_phase(Boss* b);
void boss_create_kira_particle(Object* b);

struct Enemy : Object {
	Coroutine co;
	float hp;
	int drops;

	float angle;

	u32 sprite_idle;
	u32 sprite_right;
	float facing = 1;

	void (*death_callback)(Object*);
	void (*update_callback)(Object*, float);
};

enum BulletType {
	BULLET_TYPE_BULLET = 1,
	BULLET_TYPE_LAZER,
};

#define BULLET_SPAWN_PARTICLE_LIFESPAN 10.0f

struct Bullet : Object {
	Coroutine co;
	instance_id owner;
	BulletType bullet_type;
	float lifetime;
	float lifespan = 10.0f * 60.0f;

	union {
		struct {
			float target_length;
			float thickness;
			float time;
			float length;
			float timer; // @Todo: use lifetime?
		} lazer;
	};
};

enum PlayerBulletType {
	PLAYER_BULLET_REIMU_CARD,
	PLAYER_BULLET_REIMU_ORB_SHOT,
};

struct PlayerBullet : Object {
	PlayerBulletType type;
	float dmg;
	// instance_id owner;

	union {
		struct {
			float rotation;
		} reimu_card;
	};
};

enum PickupType {
	PICKUP_TYPE_POWER,
	PICKUP_TYPE_POINT,
	PICKUP_TYPE_POWER_BIG,
	PICKUP_TYPE_POINT_BIG,
	PICKUP_TYPE_BOMB,
	PICKUP_TYPE_LIFE,
	PICKUP_TYPE_SCORE,
	PICKUP_TYPE_FULL_POWER,

	PICKUP_TYPE_COUNT,
};

struct Pickup : Object {
	PickupType pickup_type;
	float hsp;
	float vsp;
	instance_id homing_target;
};

void object_cleanup(Player* p);
void object_cleanup(Boss* b);
void object_cleanup(Enemy* e);
void object_cleanup(Bullet* b);
void object_cleanup(PlayerBullet* b);
void object_cleanup(Pickup* p);
