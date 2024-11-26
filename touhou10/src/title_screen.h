#pragma once

#include "common.h"

#include "menu.h"

struct Title_Screen {
	enum State {
		IN_MAIN_MENU,
		IN_OPTIONS_MENU,
	};

	State state;

	Menu menu;
	Menu options_menu;

	void init();
	void deinit();
	void update(float delta);
	void draw(float delta);
};

extern Title_Screen title_screen;
