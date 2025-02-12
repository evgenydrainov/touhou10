#pragma once

#include "common.h"

#include "world.h"
#include "title_screen.h"

#define GAME_W 640
#define GAME_H 480

#define GAME_TEXTURE_SCALE 1
#define RENDER_GAME_TO_TEXTURE 1
#define DRAW_AT_FLOORED_POSITION 1

#define PLAY_AREA_W 384
#define PLAY_AREA_H 448
#define PLAY_AREA_X 32
#define PLAY_AREA_Y 16

u32 load_3d_model_from_obj_file(const char* fname, int* out_num_vertices);

struct Stats {
	int score;
	int lives = 3;
	int bombs = 3;
	int power;
	int graze;
	int points;
};

struct Game {
	enum State {
		STATE_NONE,
		STATE_TITLE_SCREEN,
		STATE_PLAYING,
	};

	State state;
	State next_state;

	Stats stats;
	u32 stage_index;
	bool skip_to_boss;
	bool skip_to_midboss;

	Mix_Music* music;

	xoshiro128plus rng_visual{{0x68756F54, 0x7250756F, 0x63656A6F, 0x35393474}};

	bool show_debug_info;
	bool frame_advance;
	bool skip_frame;

	void init();
	void deinit();

	void update(float delta);
	void draw(float delta);
	void late_draw(float delta);
};

extern Game game;

#ifdef DEVELOPER
extern array<string> g_ConsoleCommands;

bool console_callback(string str, void* userdata);
#endif
