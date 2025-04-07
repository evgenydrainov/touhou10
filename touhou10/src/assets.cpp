#include "assets.h"

#include "package.h"
#include "util.h"

static Texture    textures[NUM_TEXTURES];
static Sprite     sprites[NUM_SPRITES];
static Font       fonts[NUM_TEXTURES];
static Mix_Chunk* sounds[NUM_SOUNDS];
static Shader     shaders[NUM_SHADERS];

#define FILTER_FOR_SPRITES GL_LINEAR

void load_global_assets() {
	{
		textures[tex_characters]   = load_texture_from_file("textures/characters.png",  FILTER_FOR_SPRITES);
		textures[tex_projectiles]  = load_texture_from_file("textures/projectiles.png", FILTER_FOR_SPRITES);
		textures[tex_ui]           = load_texture_from_file("textures/ui.png",          FILTER_FOR_SPRITES);
		textures[tex_bosses]       = load_texture_from_file("textures/bosses.png",      FILTER_FOR_SPRITES);
		textures[tex_enemies]      = load_texture_from_file("textures/enemies.png",     FILTER_FOR_SPRITES);

		textures[tex_gfw_misty_lake]   = load_texture_from_file("textures/gfw_misty_lake.png",   GL_LINEAR, GL_REPEAT);
		textures[tex_gfw_misty_lake2]  = load_texture_from_file("textures/gfw_misty_lake2.png",  GL_LINEAR, GL_REPEAT);
		textures[tex_gfw_misty_lake3]  = load_texture_from_file("textures/gfw_misty_lake3.png",  GL_LINEAR, GL_REPEAT);
		textures[tex_gfw_misty_lake4]  = load_texture_from_file("textures/gfw_misty_lake4.png",  GL_LINEAR, GL_REPEAT);
		textures[tex_pcb_youmu_stairs] = load_texture_from_file("textures/pcb_youmu_stairs.png", GL_LINEAR);

		auto gen_mipmap = [](u32 texture_index) {
			u32 id = get_texture(texture_index).id;
			if (id != 0) {
				glBindTexture(GL_TEXTURE_2D, id);
				glGenerateMipmap(GL_TEXTURE_2D);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
				glBindTexture(GL_TEXTURE_2D, 0);
			}
		};

		gen_mipmap(tex_gfw_misty_lake);
		gen_mipmap(tex_gfw_misty_lake2);
		gen_mipmap(tex_gfw_misty_lake3);
		gen_mipmap(tex_gfw_misty_lake4);
		gen_mipmap(tex_pcb_youmu_stairs);

		textures[tex_cirno_spellcard_background]   = load_texture_from_file("textures/cirno_spellcard_background.png", GL_LINEAR);
		textures[tex_background]                   = load_texture_from_file("textures/background.png", GL_NEAREST, GL_REPEAT);
		textures[tex_white]                        = load_texture_from_file("textures/white.png");
		textures[tex_boss_cirno_portrait]          = load_texture_from_file("textures/boss_cirno_portrait.png");
		textures[tex_boss_youmu_portrait]          = load_texture_from_file("textures/boss_youmu_portrait.png");
		textures[tex_spellcard_attack_anim_label]  = load_texture_from_file("textures/spellcard_attack_anim_label.png");
		textures[tex_pcb_youmu_bg]                 = load_texture_from_file("textures/pcb_youmu_bg.png", GL_LINEAR);
		textures[tex_pcb_youmu_bg_flowers]         = load_texture_from_file("textures/pcb_youmu_bg_flowers.png", GL_LINEAR);
		textures[tex_title_screen_bg]              = load_texture_from_file("textures/title_screen_bg.png");
		textures[tex_heat_haze_leopard]            = load_texture_from_file("textures/heat_haze_leopard.png", GL_LINEAR, GL_REPEAT);
	}

	{
		const Texture& t = get_texture(tex_characters);

		sprites[spr_reimu_idle]                 = create_sprite(t,  0,   0, 32, 48, 16, 24,  8,  8, 0.20f,  0, 32, 48);
		sprites[spr_reimu_left]                 = create_sprite(t,  0,  48, 32, 48, 16, 24,  8,  8, 0.20f,  4, 32, 48);
		sprites[spr_reimu_right]                = create_sprite(t,  0,  96, 32, 48, 16, 24,  8,  8, 0.20f,  4, 32, 48);
		sprites[spr_reimu_shot_card]            = create_sprite(t,  0, 144, 16, 16,  8,  8);
		sprites[spr_reimu_shot_orb]             = create_sprite(t, 16, 144, 16, 16,  8,  8);
		sprites[spr_reimu_shot_card_afterimage] = create_sprite(t, 32, 144, 16, 16,  8,  8);
		sprites[spr_reimu_shot_orb_afterimage]  = create_sprite(t, 48, 144, 16, 16,  8,  8);
		sprites[spr_reimu_orb]                  = create_sprite(t, 64, 144, 16, 16,  8,  8,  3);
		sprites[spr_player_hitbox]              = create_sprite(t, 192, 144, 64, 64, 32, 32,  1,  1);
	}

	{
		const Texture& t = get_texture(tex_projectiles);

		sprites[spr_lazer]                     = create_sprite(t,   0,   0,  16,  16,  8,  0, 16);
		sprites[spr_bullet_arrow]              = create_sprite(t,   0,  16,  16,  16,  8,  8, 16);
		sprites[spr_bullet_outline]            = create_sprite(t,   0,  32,  16,  16,  8,  8, 16);
		sprites[spr_bullet_filled]             = create_sprite(t,   0,  48,  16,  16,  8,  8, 16);
		sprites[spr_bullet_rice]               = create_sprite(t,   0,  64,  16,  16,  8,  8, 16);
		sprites[spr_bullet_kunai]              = create_sprite(t,   0,  80,  16,  16,  8,  8, 16);
		sprites[spr_bullet_shard]              = create_sprite(t,   0,  96,  16,  16,  8,  8, 16);
		sprites[spr_bullet_card]               = create_sprite(t,   0, 112,  16,  16,  8,  8, 16);
		sprites[spr_bullet_bullet]             = create_sprite(t,   0, 128,  16,  16,  8,  8, 16);
		sprites[spr_bullet_small]              = create_sprite(t,  64, 176,   8,   8,  4,  4, 16,  8);
		sprites[spr_bullet_spawn_particle]     = create_sprite(t,   0, 144,  32,  32, 16, 16,  8);
		sprites[spr_enemy_death_particle_blue] = create_sprite(t,   0, 176,  64,  64, 32, 32);
		sprites[spr_kira_particle]             = create_sprite(t,  64, 192,  32,  32, 16, 16);
		sprites[spr_pickup]                    = create_sprite(t, 128, 176,  16,  16,  8,  8, 16,  8);
		sprites[spr_particle_graze]            = create_sprite(t,  64, 224,   8,   8,  4,  4,  4,  4, 0.25f);
		sprites[spr_boss_pentagram]            = create_sprite(t,   0, 240, 128, 128, 64, 64);
	}

	{
		const Texture& t = get_texture(tex_ui);

		sprites[spr_enemy_label] = create_sprite(t, 0, 0, 48, 16, 24, 0);
	}

	{
		const Texture& t = get_texture(tex_bosses);

		sprites[spr_boss_cirno_idle]  = create_sprite(t,   0,   0, 64, 64, 32, 32,  4,  4, 0.15f);
		sprites[spr_boss_cirno_right] = create_sprite(t,   0,  64, 64, 64, 32, 32,  4,  4, 0.15f,  3);
		sprites[spr_boss_cirno_left]  = create_sprite(t,   0, 128, 64, 64, 32, 32,  4,  4, 0.15f,  3);
		sprites[spr_boss_youmu_idle]  = create_sprite(t, 256,   0, 48, 64, 24, 32,  4,  4, 0.10f);
		sprites[spr_boss_youmu_left]  = create_sprite(t, 256,  64, 48, 64, 24, 32,  4,  4, 0.15f,  3);
		sprites[spr_boss_youmu_right] = create_sprite(t, 256, 128, 48, 64, 24, 32,  4,  4, 0.15f,  3);
	}

	{
		const Texture& t = get_texture(tex_enemies);

		sprites[spr_boss_daiyousei_idle] = create_sprite(t,   0,  64, 48, 48, 24, 24, 1);
		sprites[spr_fairy_0]             = create_sprite(t,   0,   0, 32, 32, 16, 16, 8,  8, 0.10f);
		sprites[spr_fairy_1]             = create_sprite(t,   0,  32, 32, 32, 16, 16, 8,  8, 0.10f);
		sprites[spr_enemy_0]             = create_sprite(t,  48,  64, 32, 32, 16, 16, 1);
	}

	{
		fonts[fnt_consolas_bold] = load_bmfont_file("fonts/consolas_bold.fnt", "fonts/consolas_bold_0.png");
		fonts[fnt_cirno]         = load_bmfont_file("fonts/cirno.fnt",         "fonts/cirno_0.png");
		fonts[fnt_indigo_hunter] = load_bmfont_file("fonts/indigo_hunter.fnt", "fonts/indigo_hunter_0.png");

		fonts[fnt_main] = load_font_from_texture("fonts/fnt_main.png", 15, 15, 15, 16, 16);
	}

	{
		sounds[snd_boss_die]         = load_sound("sounds/boss_die.wav");
		sounds[snd_char_reimu_shoot] = load_sound("sounds/char_reimu_shoot.wav");
		sounds[snd_enemy_die]        = load_sound("sounds/enemy_die.wav");
		sounds[snd_enemy_hurt]       = load_sound("sounds/enemy_hurt.wav");
		sounds[snd_enemy_shoot]      = load_sound("sounds/enemy_shoot.wav");
		sounds[snd_extend]           = load_sound("sounds/extend.wav");
		sounds[snd_graze]            = load_sound("sounds/graze.wav");
		sounds[snd_lazer]            = load_sound("sounds/lazer.wav");
		sounds[snd_menu_cancel]      = load_sound("sounds/menu_cancel.wav");
		sounds[snd_menu_navigate]    = load_sound("sounds/menu_navigate.wav");
		sounds[snd_menu_ok]          = load_sound("sounds/menu_ok.wav");
		sounds[snd_pause]            = load_sound("sounds/pause.wav");
		sounds[snd_pichuun]          = load_sound("sounds/pichuun.wav");
		sounds[snd_pickup]           = load_sound("sounds/pickup.wav");
		sounds[snd_powerup]          = load_sound("sounds/powerup.wav");
		sounds[snd_spellcard]        = load_sound("sounds/spellcard.wav");
		sounds[snd_kira]             = load_sound("sounds/kira.wav");

		if (sounds[snd_enemy_shoot]) {
			Mix_VolumeChunk(sounds[snd_enemy_shoot], (int)(MIX_MAX_VOLUME * 0.50f));
		}
	}

	reload_shaders();
}

void reload_shaders() {
	for (int i = 0; i < NUM_SHADERS; i++) {
		free_shader(&shaders[i]);
	}

	{
		shaders[shd_stage0_bg] = load_shader_from_file("shaders/stage0_bg.vert", "shaders/stage0_bg.frag");
		shaders[shd_basic_3d]  = load_shader_from_file("shaders/basic_3d.vert",  "shaders/basic_3d.frag");
		shaders[shd_heat_haze] = load_shader_from_file("shaders/heat_haze.vert", "shaders/heat_haze.frag");
	}
}

void free_all_assets() {
	for (int i = 0; i < NUM_TEXTURES; i++) {
		free_texture(&textures[i]);
	}
	
	for (int i = 0; i < NUM_FONTS; i++) {
		free_font(&fonts[i]);
	}

	for (int i = 0; i < NUM_SOUNDS; i++) {
		if (sounds[i]) {
			Mix_FreeChunk(sounds[i]);
			sounds[i] = nullptr;
		}
	}

	for (int i = 0; i < NUM_SHADERS; i++) {
		free_shader(&shaders[i]);
	}
}

const Texture& get_texture(u32 texture_index) {
	Assert(texture_index < NUM_TEXTURES);

	if (textures[texture_index].id == 0) {
		log_warn("Trying to access texture %u that hasn't been loaded.", texture_index);
	}

	return textures[texture_index];
}

const Sprite& get_sprite(u32 sprite_index) {
	Assert(sprite_index < NUM_SPRITES);

	if (sprites[sprite_index].frames.count == 0) {
		log_warn("Trying to access sprite %u that hasn't been loaded.", sprite_index);
	}

	return sprites[sprite_index];
}

const Font& get_font(u32 font_index) {
	Assert(font_index < NUM_FONTS);

	if (fonts[font_index].glyphs.count == 0) {
		log_warn("Trying to access font %u that hasn't been loaded.", font_index);
	}

	return fonts[font_index];
}

Mix_Chunk* get_sound(u32 sound_index) {
	Assert(sound_index < NUM_SOUNDS);

	if (!sounds[sound_index]) {
		log_warn("Trying to access sound %u that hasn't been loaded.", sound_index);
	}

	return sounds[sound_index];
}

const Shader& get_shader(u32 shader_index) {
	Assert(shader_index < NUM_SHADERS);

	if (shaders[shader_index].id == 0) {
		log_warn("Trying to access shader %u that hasn't been loaded.", shader_index);
	}

	return shaders[shader_index];
}
