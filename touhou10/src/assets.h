#pragma once

#include "common.h"
#include "sprite_indices.h"
#include "animation.h"

#include <minicoro/minicoro.h>
#include <SDL_mixer.h>

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
	tex_eosd_misty_lake,
	tex_cirno_spellcard_background,
	tex_background,
	tex_white,
	tex_boss_cirno_portrait,
	tex_boss_youmu_portrait,
	tex_spellcard_attack_anim_label,
	tex_pcb_youmu_stairs,
	tex_pcb_youmu_bg,
	tex_pcb_youmu_bg_flowers,

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
	int loop_frame; // The frame from which animation will loop.
	float anim_spd; // Animation speed
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
// Boss data
// 

enum PhaseType {
	PHASE_NONSPELL,
	PHASE_SPELLCARD,
};

struct BossPhase {
	string name;
	float hp;
	float time;
	PhaseType type;
	void (*script)(mco_coro*);
};

enum BossType {
	BOSS_TYPE_BOSS,
	BOSS_TYPE_MIDBOSS,
};

struct BossData {
	const char* name;
	BossPhase* phases;
	int phase_count;
	BossType type;
	u32 spr_idle;
	u32 spr_left;
	u32 spr_right;
	void (*draw_spellcard_background)(float delta);
	u32 portrait_texture; // @Temp? The portrait used in spellcard animation.
};

#define BOSS_INDEX_ENUM(X) \
	X(BOSS_CIRNO) \
	X(BOSS_DAIYOUSEI_MIDBOSS) \
	X(BOSS_YOUMU_MIDBOSS) \
	\
	X(NUM_BOSSES)

DEFINE_NAMED_ENUM(BossIndex, BOSS_INDEX_ENUM)

extern BossData boss_data[NUM_BOSSES];

BossData* GetBossData(u32 boss_index);

// 
// Stage data
// 

struct StageData {
	void (*script)(mco_coro*);
	void (*init_background)();
	void (*draw_background)(float delta);
};

#define STAGE_COUNT 2

extern StageData stage_data[STAGE_COUNT];

StageData* GetStageData(u32 stage_index);


// 
// Sounds
// 

enum {
	snd_boss_die,
	snd_char_reimu_shoot,
	snd_enemy_die,
	snd_enemy_hurt,
	snd_enemy_shoot,
	snd_extend,
	snd_graze,
	snd_lazer,
	snd_menu_cancel,
	snd_menu_navigate,
	snd_menu_ok,
	snd_pause,
	snd_pichuun,
	snd_pickup,
	snd_powerup,
	snd_spellcard,

	NUM_SOUNDS,
};

extern Mix_Chunk* sound_data[NUM_SOUNDS];

Mix_Chunk* GetSound(u32 sound_index);

// 
// Animations
// 

extern AnimData anim_boss_spellcard;
