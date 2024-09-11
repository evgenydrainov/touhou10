#pragma once

#include "common.h"

enum MenuState {
	MENU_ANIM_IN,
	MENU_NORMAL,
	MENU_ANIM_OUT,
	MENU_DONE,
};

enum {
	MENU_NO_FADEOUT    = 1,
	MENU_DRAW_CENTERED = 1 << 1,
};

struct Menu {
	MenuState state;
	float animation;
	int cursor;
};

int menu_update(Menu* menu, int num_items, float delta, u32 flags = 0);

void menu_draw(Menu* menu, int num_items,
			   float menu_start_x, float menu_start_y,
			   float sep_x, float sep_y,
			   string* labels, u32 flags = 0);
