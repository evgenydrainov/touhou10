#pragma once

#include "common.h"
#include "sprite_indices.h"
#include <minicoro/minicoro.h>

struct Player;

// 
// Textures
// 

struct Texture {
	u32 ID;
	int width;
	int height;
};

enum {
	tex_atlas_0,
	tex_stage_0_bg,
	tex_cirno_spellcard_background,
	tex_background,

	NUM_TEXTURES,
};

extern Texture texture_data[NUM_TEXTURES];

Texture* GetTexture(u32 texture_index);

// 
// Sprites
// 

struct SpriteFrame {
	int u;
	int v;
	int w;
	int h;
};

struct Sprite {
	u32 texture_index;
	SpriteFrame* frames;
	int frame_count;
	int xorigin;
	int yorigin;
	int loop_frame;
	float anim_spd;
	int width;
	int height;
};

extern Sprite sprite_data[NUM_SPRITES];

Sprite* GetSprite(u32 sprite_index);

// 
// Characters
// 

struct Character {
	const char* name;
	float move_spd;
	float focus_spd;
	float radius;
	float graze_radius;
	float deathbomb_time;
	int starting_bombs;
	void (*shot_type)(Player* player_index, float delta);
	void (*bomb)(Player* player_index);
	u32 spr_idle;
	u32 spr_left;
	u32 spr_right;
};

enum {
	CHARACTER_REIMU,

	NUM_CHARACTERS,
};

extern Character character_data[NUM_CHARACTERS];

Character* GetCharacter(u32 character_index);

// 
// Shaders
// 

extern const char shader_texture_vertex_text[];
extern const char shader_texture_fragment_text[];

extern const char shader_color_fragment_text[];

extern const char shader_stage_0_bg_vertex_text[];
extern const char shader_stage_0_bg_fragment_text[];

// 
// Boss data
// 

enum PhaseType {
	PHASE_NONSPELL,
	PHASE_SPELLCARD,
};

struct BossPhase {
	const char* name;
	float hp;
	float time;
	PhaseType type;
	void (*script)(mco_coro*);
};

struct BossData {
	const char* name;
	BossPhase* phases;
	int phase_count;
	bool midboss;
	u32 spr_idle;
	u32 spr_left;
	u32 spr_right;
	void (*draw_spellcard_background)(float delta);
};

#define BOSS_INDEX_ENUM(X) \
	X(BOSS_CIRNO) \
	X(MIDBOSS_DAIYOUSEI) \
	X(MIDBOSS_YOUMU) \
	\
	X(NUM_BOSSES)

DEFINE_NAMED_ENUM(BossIndex, BOSS_INDEX_ENUM)

extern BossData boss_data[];

BossData* GetBossData(u32 boss_index);

// 
// Stage data
// 

struct StageData {
	void (*script)(mco_coro*);
	void (*draw_background)(float delta);
};

extern StageData stage_data[];

StageData* GetStageData(u32 stage_index);
