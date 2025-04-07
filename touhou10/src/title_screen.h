#pragma once

#include "common.h"

struct Title_Screen {
	enum State {
		IN_MAIN_MENU,
		IN_OPTIONS_MENU,
	};

	enum MenuState {
		MENU_FADEIN,
		MENU_NORMAL,
		MENU_BLINK,
		MENU_FADEOUT,
		MENU_DONE,
	};

	struct Label {
		string text;
		float x;
		float y;
		vec4 color;

		bool animating;
		float anim_t;
	};

	State state;

	MenuState main_menu_state;
	array<Label> main_menu_labels;
	int main_menu_cursor;
	float main_menu_t;
	float main_menu_blink_t;

	void init();
	void deinit();

	void update(float delta);
	void update_main_menu(float delta);
	void update_options_menu(float delta);

	void draw(float delta);
	void draw_main_menu(float delta);
	void draw_options_menu(float delta);
};

extern Title_Screen title_screen;
