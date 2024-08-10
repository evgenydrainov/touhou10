#include "title_screen.h"

#include "game.h"
#include <glad/gl.h>

#define MENU_SIZE 3

void Title_Screen::init() {}

void Title_Screen::destroy() {}

void Title_Screen::update(float delta) {

	auto navigate_and_select = [&]() {
		if (is_key_pressed(SDL_SCANCODE_UP)) {
			if (cursor > 0) {
				cursor--;
				play_sound(snd_menu_navigate);
			}
		}

		if (is_key_pressed(SDL_SCANCODE_DOWN)) {
			if (cursor < MENU_SIZE - 1) {
				cursor++;
				play_sound(snd_menu_navigate);
			}
		}

		if (is_key_pressed(SDL_SCANCODE_X)) {
			cursor = MENU_SIZE - 1;
			play_sound(snd_menu_cancel);
		}

		if (is_key_pressed(SDL_SCANCODE_Z)) {
			state = ANIM_OUT;
			animation = 1;
			play_sound(snd_menu_ok);
		}
	};

	const float anim_spd = 1.0f / 30.0f;

	switch (state) {
		case ANIM_IN: {
			animation = approach(animation, 1.0f, anim_spd * delta);

			if (animation == 1) {
				state = NORMAL;
			}

			navigate_and_select();
			break;
		}

		case NORMAL: {
			navigate_and_select();
			break;
		}

		case ANIM_OUT: {
			animation = approach(animation, 0.0f, anim_spd * delta);

			if (animation == 0) {
				switch (cursor) {
					case 0: {
						g->stage_index     = 0;
						g->skip_to_boss    = false;
						g->skip_to_midboss = false;
						g->next_state      = Game::STATE_PLAYING;
						break;
					}
					case 1: {
						state = NORMAL; // @Temp
						animation = 1;
						break;
					}
					case 2: {
						g->quit = true;
						break;
					}
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

	string labels[MENU_SIZE] = {"Play", "Options", "Quit"};

	for (int i = 0; i < ArrayLength(labels); i++) {
		const float menu_start_x = GAME_W * 0.75f;
		const float menu_start_y = GAME_H * 0.55f;

		const float sep_x = 20;
		const float sep_y = 50;

		float x = menu_start_x - sep_x * i + 100 * (1 - animation); // slide from the left
		float y = menu_start_y + sep_y * i;

		vec4 color = color_white;
		if (i == cursor) {
			color = color_yellow;
		}

		r->draw_text(GetSprite(spr_font_main), labels[i], x, y, HALIGN_LEFT, VALIGN_TOP, color);
	}

	r->break_batch();
}
