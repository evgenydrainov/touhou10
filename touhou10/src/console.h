#pragma once

#include "common.h"

union SDL_Event;

struct Console {
	static constexpr size_t COMMAND_HISTORY = 5;

	bool show;

	static char user_input_line_buf[64];
	dynamic_array_cap<char> user_input_line;

	int caret;

	static char command_history_buf[COMMAND_HISTORY][64];
	dynamic_array_cap<char> command_history[COMMAND_HISTORY];

	int history_index = -1;

	static char history_buf[1024];
	dynamic_array_cap<char> history;

	float scroll;

	void init();
	void destroy();

	void update(float delta);
	void event(SDL_Event* ev);
	void execute();

	void write(char ch);
	void write(string str);

	void draw(float delta);
};
