#pragma once

#include "common.h"
#include "renderer.h"
#include "xoshiro128plus.h"

#define GAME_W 640
#define GAME_H 480

#define PLAY_AREA_X 32
#define PLAY_AREA_Y 16

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

	Framebuffer stage_bg_fbo;

	xoshiro128plus rng_visual{{0x68756F54, 0x7250756F, 0x63656A6F, 0x35393474}};

	bool show_debug_info;

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
