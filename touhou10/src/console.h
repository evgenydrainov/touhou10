#pragma once

#include <SDL.h>

struct Console {
	bool show;
	char cmd[32];
	size_t caret;
	static char text[1024];
	size_t text_pos;
	float scroll;
	char prev_cmd[32];
	size_t prev_caret;

	void update(float delta);
	void event(SDL_Event* ev);
	void execute();
	void write(char ch);
	void write(const char* buf, size_t size);
	void write(const char* buf);

	void draw(float delta);
};
