#pragma once

#include "common.h"

enum MenuState {
	MENU_ANIM_IN,
	MENU_NORMAL,
	MENU_ANIM_OUT,
	MENU_DONE,
};

struct Menu {
	MenuState state;
	float animation;
	int cursor;
	int num_items;
};

int menu_update(Menu* menu, float delta);

struct Title_Screen {
	enum State {
		IN_MAIN_MENU,
		IN_OPTIONS_MENU,
	};

	State state;

	Menu menu;
	Menu options_menu;

	void init();
	void destroy();
	void update(float delta);
	void draw(float delta);
};
