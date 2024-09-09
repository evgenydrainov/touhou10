#include "title_screen.h"

#include "game.h"
#include <glad/gl.h>

#define MENU_SIZE         3
#define OPTIONS_MENU_SIZE 1

void Title_Screen::init() {
	menu.num_items = MENU_SIZE;
	options_menu.num_items = OPTIONS_MENU_SIZE;
}

void Title_Screen::destroy() {}

void Title_Screen::update(float delta) {
	switch (state) {
		case IN_MAIN_MENU: {
			int cursor = menu_update(&menu, delta);

			switch (cursor) {
				case 0: {
					g->stage_index     = 0;
					g->skip_to_boss    = false;
					g->skip_to_midboss = false;
					g->next_state      = Game::STATE_PLAYING;
					break;
				}
				case 1: {
					state = IN_OPTIONS_MENU;
					options_menu = {};
					options_menu.num_items = OPTIONS_MENU_SIZE;
					break;
				}
				case 2: {
					g->quit = true;
					break;
				}
			}
			break;
		}

		case IN_OPTIONS_MENU: {
			int cursor = menu_update(&menu, delta);

			switch (cursor) {
				case 0: {
					break;
				}
			}
			break;
		}
	}
}

void Title_Screen::draw(float delta) {
	glClearColor(0, 0, 0, 1);
	glClear(GL_COLOR_BUFFER_BIT);

	glViewport(g->game_viewport.x, g->game_viewport.y, g->game_viewport.w, g->game_viewport.h);
	r->proj = glm::ortho<float>(0, GAME_W, GAME_H, 0);

	r->draw_text(GetSprite(spr_font_main), "The Title Screen.", 16, 16);

	switch (state) {
		case IN_MAIN_MENU: {
			string labels[MENU_SIZE] = {"Play", "Options", "Quit"};

			for (int i = 0; i < ArrayLength(labels); i++) {
				const float menu_start_x = GAME_W * 0.75f;
				const float menu_start_y = GAME_H * 0.55f;

				const float sep_x = 20;
				const float sep_y = 50;

				float x = menu_start_x - sep_x * i;
				float y = menu_start_y + sep_y * i;

				// Animate.
				float xoff = 100 * (1 - menu.animation);
				if (i % 2) xoff = -xoff;

				x += xoff;

				vec4 color = color_white;
				if (i == menu.cursor) {
					color = color_yellow;
				}

				r->draw_text(GetSprite(spr_font_main), labels[i], x, y, HALIGN_LEFT, VALIGN_TOP, color);
			}
			break;
		}

		case IN_OPTIONS_MENU: {
			string labels[OPTIONS_MENU_SIZE] = {"Volume"};

			for (int i = 0; i < ArrayLength(labels); i++) {
				float x = 50;
				float y = 50;

				vec4 color = color_white;
				if (i == menu.cursor) {
					color = color_yellow;
				}

				r->draw_text(GetSprite(spr_font_main), labels[i], x, y, HALIGN_LEFT, VALIGN_TOP, color);
			}
			break;
		}
	}

	r->break_batch();
}

int menu_update(Menu* menu, float delta) {
	auto navigate_and_select = [&]() {
		if (is_key_pressed(SDL_SCANCODE_UP)) {
			if (menu->cursor > 0) {
				menu->cursor--;
				play_sound(snd_menu_navigate);
			}
		}

		if (is_key_pressed(SDL_SCANCODE_DOWN)) {
			if (menu->cursor < menu->num_items - 1) {
				menu->cursor++;
				play_sound(snd_menu_navigate);
			}
		}

		if (is_key_pressed(SDL_SCANCODE_X)) {
			menu->cursor = menu->num_items - 1;
			play_sound(snd_menu_cancel);
		}

		if (is_key_pressed(SDL_SCANCODE_Z)) {
			menu->state = MENU_ANIM_OUT;
			menu->animation = 1;
			play_sound(snd_menu_ok);
		}
	};

	int result = -1;

	const float anim_spd = 1.0f / 30.0f;

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
