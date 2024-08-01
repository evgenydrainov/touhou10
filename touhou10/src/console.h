#pragma once

#include "common.h"

union SDL_Event;

struct Console {
	bool show;

	char user_input_line_buf[32];
	dynamic_array_cap<char> user_input_line;

	char user_input_line_prev_buf[32];
	dynamic_array_cap<char> user_input_line_prev;

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
