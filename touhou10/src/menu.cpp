#include "menu.h"

#include "game.h"

int menu_update(Menu* menu, int num_items, float delta, u32 flags) {
	int result = -1;

	auto navigate_and_select = [&]() {
		if (is_key_pressed(SDL_SCANCODE_UP)) {
			if (menu->cursor > 0) {
				menu->cursor--;
				play_sound(snd_menu_navigate);
			}
		}

		if (is_key_pressed(SDL_SCANCODE_DOWN)) {
			if (menu->cursor < num_items - 1) {
				menu->cursor++;
				play_sound(snd_menu_navigate);
			}
		}

		if (is_key_pressed(SDL_SCANCODE_X)) {
			menu->cursor = num_items - 1;
			play_sound(snd_menu_cancel);
		}

		if (is_key_pressed(SDL_SCANCODE_Z)) {
			if (flags & MENU_NO_FADEOUT) {
				result = menu->cursor;
			} else {
				menu->state = MENU_ANIM_OUT;
				menu->animation = 1;
				play_sound(snd_menu_ok);
			}
		}
	};

	const float anim_spd = 1.0f / 15.0f;

	switch (menu->state) {
		case MENU_ANIM_IN: {
			menu->animation = approach(menu->animation, 1.0f, anim_spd * delta);

			if (menu->animation == 1) {
				menu->state = MENU_NORMAL;
			}
			break;
		}

		case MENU_NORMAL: {
			navigate_and_select();
			break;
		}

		case MENU_ANIM_OUT: {
			menu->animation = approach(menu->animation, 0.0f, anim_spd * delta);

			if (menu->animation == 0) {
				result = menu->cursor;
				menu->state = MENU_DONE;
			}
			break;
		}
	}

	return result;
}

void menu_draw(Menu* menu, int num_items,
			   float menu_start_x, float menu_start_y,
			   float sep_x, float sep_y,
			   string* labels, u32 flags) {
	for (int i = 0; i < num_items; i++) {
		float x = menu_start_x - sep_x * i;
		float y = menu_start_y + sep_y * i;

		// Animate.
		float xoff = 50 * (1 - menu->animation);
		if (i % 2) xoff = -xoff;

		x += xoff;

		vec4 color = color_white;
		if (i == menu->cursor) {
			color = color_yellow;
		}

		HAlign halign = HALIGN_LEFT;
		if (flags & MENU_DRAW_CENTERED) {
			halign = HALIGN_CENTER;
		}

		r->draw_text(GetSprite(spr_font_main), labels[i], x, y, halign, VALIGN_TOP, color);
	}
}
