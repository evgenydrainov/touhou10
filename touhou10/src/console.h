#pragma once

#include "common.h"
#include "strings.h"

union SDL_Event;

struct Console {
	bool show;

	Static_String<32> user_input_line;
	Static_String<32> user_input_line_prev;

	static Static_String<1024> history;

	float scroll;

	void update(float delta);
	void event(SDL_Event* ev);
	void execute();

	void write(char ch);
	void write(String str);

	void draw(float delta);
};
