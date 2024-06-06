#pragma once

#include "common.h"
#include "assets.h"
#include <minicoro/minicoro.h>

typedef u64 instance_id;

enum ObjType : u64 {
	OBJ_TYPE_PLAYER = 1,
	OBJ_TYPE_BOSS,
	OBJ_TYPE_BULLET,
	OBJ_TYPE_PLAYER_BULLET,
};

enum {
	FLAG_INSTANCE_DEAD = 1,

	FLAG_BULLET_ROTATE = 1 << 1,
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

	Sprite* GetSprite() {
		return ::GetSprite(sprite_index);
	}
};

void object_init(Object* o, ObjType type);
void object_animate(Object* o, float delta);

struct Player : Object {
	float hsp;
	float vsp;

	u32 character_index;
	bool focused;
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
void player_draw(Player* p, float delta);

#define BOSS_STATE_ENUM(X) \
	X(BOSS_STATE_NORMAL) \
	X(BOSS_WAITING_FOR_PHASE_TO_START) \
	X(BOSS_WAITING_FOR_SPELL_TO_END)

DEFINE_NAMED_ENUM(BossState, BOSS_STATE_ENUM)

struct Boss : Object {
	u32 boss_index;
	BossState state;
	int phase_index;
	float hp;
	float timer;
	float wait_timer;

	mco_coro* co;

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

struct Bullet : Object {
	mco_coro* co;
	instance_id owner;
};

enum PlayerBulletType {
	PLAYER_BULLET_REIMU_CARD,
	PLAYER_BULLET_REIMU_ORB_SHOT,
};

struct PlayerBullet : Object {
	PlayerBulletType type;
	float dmg;
	// instance_id owner;
};

void object_cleanup(Bullet* b);
void object_cleanup(Boss* b);
void object_cleanup(PlayerBullet* b);
void object_cleanup(Player* p);
