#pragma once

#include "common.h"
#include <SDL.h>

#include "arena.h"
#include "renderer.h"
#include "world.h"
#include "console.h"

#define GAME_W 640
#define GAME_H 480

#define PLAY_AREA_W 384
#define PLAY_AREA_H 448
#define PLAY_AREA_X 32
#define PLAY_AREA_Y 16

struct Game;
extern Game* g;

Texture load_texture(const char* fname, bool filter = false);

double GetTime();

struct Game {
	SDL_Window* window;

	Renderer renderer;

	u32 game_fbo;
	u32 game_texture;

	SDL_GLContext gl_context;

	World world;

	Arena arena;
	Arena frame_arena;

	u32 key_pressed[(SDL_SCANCODE_UP + 1 + 31) / 32];
	u32 key_repeat[(SDL_SCANCODE_UP + 1 + 31) / 32];

	double fps;
	double update_took;
	double draw_took;

	bool show_debug_info;
	bool frame_advance;
	bool skip_frame;

	Console console;

	void init();
	void destroy();
	void run();

	void update(float delta);
	void draw(float delta);
};

static bool is_key_pressed(SDL_Scancode key, bool repeat = true) {
	bool result = false;

	if (!g->console.show) {
		result |= (g->key_pressed[key / 32] & (1 << (key % 32))) != 0;

		if (repeat) {
			result |= (g->key_repeat[key / 32] & (1 << (key % 32))) != 0;
		}
	}

	return result;
}

static bool is_key_held(SDL_Scancode key) {
	bool result = false;

	if (!g->console.show) {
		const u8* state = SDL_GetKeyboardState(nullptr);

		result |= (state[key] != 0);
	}

	return result;
}
