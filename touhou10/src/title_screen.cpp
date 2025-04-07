#include "title_screen.h"

#include "window_creation.h"
#include "renderer.h"
#include "game.h"
#include "assets.h"
#include "input.h"

Title_Screen title_screen;

void Title_Screen::init() {
	main_menu_labels = calloc_array<Label>(5);

	main_menu_labels[0].text = "Play";
	main_menu_labels[1].text = "Extra";
	main_menu_labels[2].text = "Practice";
	main_menu_labels[3].text = "Options";
	main_menu_labels[4].text = "Quit";
}

void Title_Screen::deinit() {
	free(main_menu_labels.data);
}

void Title_Screen::update(float delta) {
	if (state == IN_MAIN_MENU) {
		update_main_menu(delta);
	} else if (state == IN_OPTIONS_MENU) {
		update_options_menu(delta);
	}
}

void Title_Screen::update_main_menu(float delta) {
	// animate
	{
		float spd = 1.0f / 15.0f;
		float blink_spd = 1.0f / 15.0f;

		switch (main_menu_state) {
			case MENU_FADEIN: {
				main_menu_t = approach(main_menu_t, 1.0f, spd * delta);

				if (main_menu_t == 1) {
					main_menu_state = MENU_NORMAL;
				}
				break;
			}

			case MENU_NORMAL: {
				break;
			}

			case MENU_BLINK: {
				main_menu_blink_t = approach(main_menu_blink_t, 1.0f, blink_spd * delta);

				if (main_menu_blink_t == 1) {
					main_menu_state = MENU_FADEOUT;
					main_menu_blink_t = 0;
				}
				break;
			}

			case MENU_FADEOUT: {
				main_menu_t = approach(main_menu_t, 0.0f, spd * delta);

				if (main_menu_t == 0) {
					main_menu_state = MENU_DONE;
				}
				break;
			}
		}
	}

	// update labels
	for (int i = 0; i < main_menu_labels.count; i++) {
		auto& it = main_menu_labels[i];

		const float menu_start_x = GAME_W * 0.08;
		const float menu_start_y = GAME_H * 0.45;

		float xto = menu_start_x + 15 * i;
		float xfrom = xto - 140;

		float y = menu_start_y + 40 * i;

		float tfrom = 0.15 * i;
		if (state == MENU_FADEIN) {
			tfrom = 0.15 * (main_menu_labels.count - i);
		}
		float t = fminf((main_menu_t - tfrom) / (1 - main_menu_labels.count * 0.15), 1);

		it.x = lerp(xfrom, xto, t);
		it.y = y;

		it.color = lerp(vec4{1,1,1,0}, vec4{1,1,1,1}, t);

		if (it.animating) {
			it.anim_t = approach(it.anim_t, 1.0f, (1.0f / 10.0f) * delta);
			it.x += sinf(it.anim_t * 2 * PI) * 3;

			if (it.anim_t == 1) it.animating = false;
		}
	}

	// navigate
	if (main_menu_state == MENU_FADEIN || main_menu_state == MENU_NORMAL) {
		if (is_input_pressed(INPUT_UI_UP, true)) {
			if (main_menu_cursor > 0) {
				main_menu_cursor--;

				main_menu_labels[main_menu_cursor].animating = true;
				main_menu_labels[main_menu_cursor].anim_t = 0;

				play_sound(get_sound(snd_menu_navigate));
			}
		}

		if (is_input_pressed(INPUT_UI_DOWN, true)) {
			if (main_menu_cursor < main_menu_labels.count - 1) {
				main_menu_cursor++;

				main_menu_labels[main_menu_cursor].animating = true;
				main_menu_labels[main_menu_cursor].anim_t = 0;

				play_sound(get_sound(snd_menu_navigate));
			}
		}

		if (is_input_pressed(INPUT_UI_CANCEL)) {
			main_menu_cursor = main_menu_labels.count - 1;

			main_menu_labels[main_menu_cursor].animating = true;
			main_menu_labels[main_menu_cursor].anim_t = 0;

			play_sound(get_sound(snd_menu_cancel));
		}

		if (is_input_pressed(INPUT_UI_CONFIRM)) {
			main_menu_state = MENU_BLINK;
			main_menu_blink_t = 0;

			play_sound(get_sound(snd_menu_ok));
		}
	}

	// perform action
	if (main_menu_state == MENU_DONE) {
		switch (main_menu_cursor) {
			case 0: {
				game.stage_index     = 0;
				game.skip_to_boss    = false;
				game.skip_to_midboss = false;
				game.next_state      = Game::STATE_PLAYING;
				break;
			}

			/*case 3: {
				state = IN_OPTIONS_MENU;
				break;
			}*/

			case 4: {
				window.should_quit = true;
				break;
			}

			default: {
				main_menu_state = MENU_FADEIN;
				break;
			}
		}
	}
}

void Title_Screen::update_options_menu(float delta) {
}

void Title_Screen::draw(float delta) {
	draw_texture(get_texture(tex_title_screen_bg));

	if (state == IN_MAIN_MENU) {
		draw_main_menu(delta);
	} else if (state == IN_OPTIONS_MENU) {
		draw_options_menu(delta);
	}
}

void Title_Screen::draw_main_menu(float delta) {
	for (int i = 0; i < main_menu_labels.count; i++) {
		auto& it = main_menu_labels[i];
		vec4 color = it.color;

		if (i == main_menu_cursor) {
			color.r *= color_yellow.r;
			color.g *= color_yellow.g;
			color.b *= color_yellow.b;

			if (main_menu_state == MENU_BLINK || main_menu_state == MENU_FADEOUT) {
				if (SDL_GetTicks() % 120 > 60) {
					color.r *= 0.75;
					color.g *= 0.75;
					color.b *= 0.75;
				}
			}
		}

		draw_text_shadow(get_font(fnt_indigo_hunter), it.text, {it.x, it.y}, HALIGN_LEFT, VALIGN_TOP, color);
	}
}

void Title_Screen::draw_options_menu(float delta) {
	/*static_assert(OPTIONS_MENU_SIZE == 3);
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
			  labels);*/
}
