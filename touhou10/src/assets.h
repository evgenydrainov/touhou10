#pragma once

#include "common.h"

#include "texture.h"
#include "font.h"
#include "sprite.h"
#include "sound_mixer.h"
#include "animation.h"

struct Player;
struct mco_coro;

enum {
	tex_characters,
	tex_projectiles,
	tex_ui,
	tex_bosses,
	tex_enemies,

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

enum {
	spr_reimu_idle,
	spr_reimu_left,
	spr_reimu_right,
	spr_reimu_shot_card,
	spr_reimu_shot_orb,
	spr_reimu_shot_card_afterimage,
	spr_reimu_shot_orb_afterimage,
	spr_reimu_orb,
	spr_player_hitbox,

	spr_lazer,
	spr_bullet_arrow,
	spr_bullet_outline,
	spr_bullet_filled,
	spr_bullet_rice,
	spr_bullet_kunai,
	spr_bullet_shard,
	spr_bullet_card,
	spr_bullet_bullet,
	spr_bullet_small,
	spr_bullet_spawn_particle,
	spr_enemy_death_particle_blue,
	spr_kira_particle,
	spr_pickup,
	spr_particle_graze,

	spr_enemy_label,

	spr_boss_cirno_idle,
	spr_boss_cirno_right,
	spr_boss_cirno_left,
	spr_boss_youmu_idle,
	spr_boss_youmu_left,
	spr_boss_youmu_right,

	spr_boss_daiyousei_idle,
	spr_fairy_0,
	spr_fairy_1,
	spr_enemy_0,

	NUM_SPRITES,
};

enum {
	fnt_consolas_bold,
	fnt_cirno,
	fnt_main,

	NUM_FONTS,
};

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
	snd_kira,

	NUM_SOUNDS,
};

enum {
	shd__,
};

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

struct StageData {
	void (*script)(mco_coro*);
	void (*init_background)();
	void (*draw_background)(float delta);
};

#define STAGE_COUNT 2

extern AnimData anim_boss_spellcard;

void load_global_assets();

void free_all_assets();

const Texture& get_texture(u32 texture_index);
const Sprite&  get_sprite(u32 sprite_index);
const Font&    get_font(u32 font_index);
Mix_Chunk*     get_sound(u32 sound_index);
Character*     GetCharacter(u32 character_index);
BossData*      GetBossData(u32 boss_index);
StageData*     GetStageData(u32 stage_index);
