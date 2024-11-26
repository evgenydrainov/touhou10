#include "title_screen.h"

#include "window_creation.h"
#include "renderer.h"
#include "game.h"

#define MENU_SIZE         3
#define OPTIONS_MENU_SIZE 3

Title_Screen title_screen;

void Title_Screen::init() {}

void Title_Screen::deinit() {}

void Title_Screen::update(float delta) {
	switch (state) {
		case IN_MAIN_MENU: {
			int cursor = menu_update(&menu, MENU_SIZE, delta);

			static_assert(MENU_SIZE == 3);
			switch (cursor) {
				case 0: {
					game.stage_index     = 0;
					game.skip_to_boss    = false;
					game.skip_to_midboss = false;
					game.next_state      = Game::STATE_PLAYING;
					break;
				}
				case 1: {
					state = IN_OPTIONS_MENU;
					options_menu = {};
					break;
				}
				case 2: {
					window.should_quit = true;
					break;
				}
			}
			break;
		}

		case IN_OPTIONS_MENU: {
			int cursor = menu_update(&options_menu, OPTIONS_MENU_SIZE, delta, MENU_NO_FADEOUT);

			static_assert(OPTIONS_MENU_SIZE == 3);
			switch (cursor) {
				case OPTIONS_MENU_SIZE - 1: {
					// @Hack???
					if (options_menu.state == MENU_DONE) {
						state = IN_MAIN_MENU;
						menu = {};
					} else {
						options_menu.state = MENU_ANIM_OUT;
						options_menu.animation = 1;
					}
					break;
				}
			}
			break;
		}
	}
}

void Title_Screen::draw(float delta) {
	renderer.proj_mat = glm::ortho<float>(0, GAME_W, GAME_H, 0);

	switch (state) {
		case IN_MAIN_MENU: {
			// Draw title screen
			//draw_text(GetSprite(spr_font_main), "The Title Screen.", 16, 16); TODO

			// Draw menu
			static_assert(MENU_SIZE == 3);
			string labels[MENU_SIZE] = {
				"Play",
				"Options",
				"Quit",
			};

			const float menu_start_x = GAME_W * 0.75f;
			const float menu_start_y = GAME_H * 0.55f;

			const float sep_x = 20;
			const float sep_y = 50;

			menu_draw(&menu, MENU_SIZE,
					  menu_start_x, menu_start_y,
					  sep_x, sep_y,
					  labels);
			break;
		}

		case IN_OPTIONS_MENU: {
			static_assert(OPTIONS_MENU_SIZE == 3);
			string labels[OPTIONS_MENU_SIZE] = {
				"Music Volume",
				"Sound Volume",
				"Back",
			};

			const float menu_start_x = GAME_W * 0.10f;
			const float menu_start_y = GAME_H * 0.25f;

			const float sep_x = 0;
			const float sep_y = 50;

			menu_draw(&options_menu, OPTIONS_MENU_SIZE,
					  menu_start_x, menu_start_y,
					  sep_x, sep_y,
					  labels);
			break;
		}
	}

	break_batch();
}
