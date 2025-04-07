#pragma once

#include "common.h"

struct Input {
	u32 state;
	u32 state_press;
	u32 state_release;
	u32 state_repeat;

	void init();
	void deinit();

	bool handle_event(const SDL_Event& ev);
	void update(float delta);
	void clear();
};

extern Input input;

// Has to match `g_InputBindings`.
enum InputKey : u32 {
	INPUT_MOVE_RIGHT    = 1 << 0,
	INPUT_MOVE_UP       = 1 << 1,
	INPUT_MOVE_LEFT     = 1 << 2,
	INPUT_MOVE_DOWN     = 1 << 3,

	INPUT_FIRE          = 1 << 4,
	INPUT_BOMB          = 1 << 5,
	INPUT_FOCUS         = 1 << 6,
	INPUT_SKIP_DIALOGUE = 1 << 7,

	INPUT_PAUSE         = 1 << 8,

	INPUT_UI_RIGHT      = 1 << 9,
	INPUT_UI_UP         = 1 << 10,
	INPUT_UI_LEFT       = 1 << 11,
	INPUT_UI_DOWN       = 1 << 12,

	INPUT_UI_CONFIRM    = 1 << 13,
	INPUT_UI_CANCEL     = 1 << 14,
};

bool is_input_held(InputKey key);
bool is_input_pressed(InputKey key, bool repeat = false);
bool is_input_released(InputKey key);
