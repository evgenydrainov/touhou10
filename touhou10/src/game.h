#pragma once

#include "common.h"

#include "renderer.h"
#include "world.h"
#include "title_screen.h"
#include "console.h"
#include "package.h"

#define GAME_W 640
#define GAME_H 480

#define GAME_TEXTURE_SCALE 1
#define RENDER_GAME_TO_TEXTURE 1
#define DRAW_AT_FLOORED_POSITION 1
#define FILTER_FOR_SPRITES GL_LINEAR

#define PLAY_AREA_W 384
#define PLAY_AREA_H 448
#define PLAY_AREA_X 32
#define PLAY_AREA_Y 16

struct Game;
extern Game* g;

// Default filter: GL_LINEAR, default wrap: GL_REPEAT
Texture    load_texture(string fname, int filter = -1, int wrap = -1);

Mix_Chunk* load_sound(string fname);
u32        load_3d_model_from_obj_file(string fname, int* out_num_vertices);

double GetTime();

void stop_sound(u32 sound_index);
void play_sound(u32 sound_index);

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

	union {
		Title_Screen title_screen = {};
		World world;
	};

	Renderer renderer;
	Console  console;
	Package  package;

	SDL_Window*   window;
	SDL_GLContext gl_context;

	u32 game_fbo;
	u32 game_texture;
	u32 game_depth_texture;

	Rect  game_texture_pos;
	float game_texture_scale;

	Rect  game_viewport;
	float game_viewport_scale;

	Rect  play_area_viewport;
	float play_area_viewport_scale;

	State state;
	State next_state;

	Stats stats;
	u32 stage_index;
	bool skip_to_boss;
	bool skip_to_midboss;

	Arena arena;
	Arena frame_arena;
	size_t arena_pos_save;

	Mix_Music* music;

	xoshiro128plus rng_visual{{0x68756F54, 0x7250756F, 0x63656A6F, 0x35393474}};

	u32 key_pressed[(SDL_SCANCODE_UP + 1 + 31) / 32];
	u32 key_repeat [(SDL_SCANCODE_UP + 1 + 31) / 32];

	double fps;
	double update_took;
	double draw_took;
	double prev_time;

	bool quit;
	bool show_debug_info;
	bool frame_advance;
	bool skip_frame;

	void init();
	void destroy();
	void run();
	void tick();

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

// Syntax sugar
inline u8* get_file(string filename, size_t* out_filesize) {
	return g->package.get_file(filename, out_filesize);
}

inline string get_file_string(string filename) {
	return g->package.get_file_string(filename);
}
