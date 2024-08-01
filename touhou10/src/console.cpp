#include "console.h"

#include "game.h"
#include <glad/gl.h>

#include <string.h>

char Console::history_buf[1024];

void Console::init() {
	user_input_line.data     = user_input_line_buf;
	user_input_line.capacity = ArrayLength(user_input_line_buf);

	user_input_line_prev.data     = user_input_line_prev_buf;
	user_input_line_prev.capacity = ArrayLength(user_input_line_prev_buf);

	history.data     = history_buf;
	history.capacity = ArrayLength(history_buf);
}

void Console::destroy() {}

void Console::update(float delta) {
	if (!show) {
		return;
	}
}

void Console::event(SDL_Event* ev) {
	switch (ev->type) {
		case SDL_KEYDOWN: {
			SDL_Scancode scancode = ev->key.keysym.scancode;

			if (show) {
				if (scancode == SDL_SCANCODE_GRAVE) {
					show = false;

				} else if (scancode == SDL_SCANCODE_BACKSPACE) {
					if (user_input_line.count > 0) {
						user_input_line.count--;
					}

				} else if (scancode == SDL_SCANCODE_RETURN) {
					memcpy(user_input_line_prev_buf, user_input_line_buf, user_input_line.count);
					user_input_line_prev.count = user_input_line.count;

					write(user_input_line);
					write('\n');

					execute();

					user_input_line.count = 0;

				} else if (scancode == SDL_SCANCODE_UP) {
					memcpy(user_input_line_buf, user_input_line_prev_buf, user_input_line_prev.count);
					user_input_line.count = user_input_line_prev.count;

				} else if (scancode == SDL_SCANCODE_DOWN) {
					user_input_line.count = 0;

				}
			} else {
				if (scancode == SDL_SCANCODE_GRAVE) {
					show = true;
					user_input_line.count = 0;
					scroll = 0;
				}
			}
			break;
		}

		case SDL_TEXTINPUT: {
			char ch = ev->text.text[0];

			if (show) {
				if (ch != '`' && ch >= 32 && ch <= 127) {
					array_add(&user_input_line, ch);
				}
			}
			break;
		}

		case SDL_MOUSEWHEEL: {
			if (show) {
				// scroll += 5 * ev->wheel.preciseY; @Todo: figure out how to scroll nicely on laptops?
				scroll += 35 * ev->wheel.y;
			}
			break;
		}
	}
}

void Console::execute() {
	String str = user_input_line;

	eat_whitespace(&str);
	String command = eat_non_whitespace(&str);

	if (command == "h" || command == "help") {
		String s = R"(Commands:
skip - Skips boss's phase
full_power - Get full power
life - Get a life
kill_player - Kills the player
)";

		write(s);
		return;
	}

	if (command == "stage") {
		eat_whitespace(&str);
		String stage_index_str = eat_non_whitespace(&str);

		bool done;
		u32 stage_index = string_to_u32(stage_index_str, &done);

		if (done && stage_index < STAGE_COUNT) {
			g->stage_index     = stage_index;
			g->skip_to_boss    = false;
			g->skip_to_midboss = false;
			g->next_state      = Game::STATE_PLAYING;

			eat_whitespace(&str);
			String token = eat_non_whitespace(&str);

			if (token == "boss") {
				g->skip_to_boss = true;
			} else if (token == "midboss") {
				g->skip_to_midboss = true;
			}
		}
		return;
	}

	if (command == "title") {
		g->next_state = Game::STATE_TITLE_SCREEN;
		return;
	}

	if (g->state == Game::STATE_PLAYING) {
		if (command == "skip") {
			if (!(w->boss.flags & FLAG_INSTANCE_DEAD)) {
				w->boss.timer = 0;
				w->boss.wait_timer = 0;
			}
			return;
		}

		if (command == "full_power") {
			get_power(MAX_POWER);
			return;
		}

		if (command == "life") {
			get_lives(1);
			return;
		}

		if (command == "kill_player") {
			w->player.state = PLAYER_STATE_DYING;
			w->player.timer = PLAYER_DEATH_TIME;
			play_sound(snd_pichuun);
			return;
		}
	}

	write("Unknown command \"");
	write(command);
	write("\"\n");
}

void Console::write(char ch) {
	if (history.count == history.capacity) {
		array_remove(&history, history.begin());
	}

	array_add(&history, ch);
}

void Console::write(String str) {
	for (size_t i = 0; i < str.count; i++) {
		write(str[i]);
	}
}

void Console::draw(float delta) {
	if (!show) {
		return;
	}

	int backbuffer_w;
	int backbuffer_h;
	SDL_GL_GetDrawableSize(g->window, &backbuffer_w, &backbuffer_h);

	int console_w = backbuffer_w;
	int console_h = (int)(backbuffer_h * 0.40f);

	r->draw_rectangle({0, 0, console_w, console_h}, {0, 0, 0, 0.5f});

	float x = 0;
	float y = console_h + scroll;

	r->break_batch();
	glScissor(0, backbuffer_h - console_h, console_w, console_h);
	glEnable(GL_SCISSOR_TEST);

	// Draw text
	r->draw_text(GetSprite(spr_font_main), history, x, y, HALIGN_LEFT, VALIGN_BOTTOM);

	// Draw shell thing
	r->draw_text(GetSprite(spr_font_main), ">", x, y, HALIGN_LEFT, VALIGN_BOTTOM);

	x += 16;

	// Draw current cmd
	r->draw_text(GetSprite(spr_font_main), user_input_line, x, y, HALIGN_LEFT, VALIGN_BOTTOM);

	r->break_batch();
	glDisable(GL_SCISSOR_TEST);
}
