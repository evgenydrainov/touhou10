#include "console.h"

#include "game.h"
#include <glad/gl.h>

#include <string.h>

char Console::history_buf[1024];
char Console::command_history_buf[Console::COMMAND_HISTORY][64];
char Console::user_input_line_buf[64];

void Console::init() {
	user_input_line.data     = user_input_line_buf;
	user_input_line.capacity = ArrayLength(user_input_line_buf);

	for (size_t i = 0; i < COMMAND_HISTORY; i++) {
		command_history[i].data     = command_history_buf[i];
		command_history[i].capacity = ArrayLength(command_history_buf[i]);
	}

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
					if (caret > 0) {
						array_remove(&user_input_line, user_input_line.begin() + caret - 1);
						caret--;
					}
				} else if (scancode == SDL_SCANCODE_DELETE) {
					if (caret < user_input_line.count) {
						array_remove(&user_input_line, user_input_line.begin() + caret);
					}
				} else if (scancode == SDL_SCANCODE_RETURN) {
					for (size_t i = COMMAND_HISTORY; i-- != 1;) {
						memcpy(command_history_buf[i], command_history_buf[i - 1], command_history[i - 1].count);
						command_history[i].count = command_history[i - 1].count;
					}

					memcpy(command_history_buf[0], user_input_line_buf, user_input_line.count);
					command_history[0].count = user_input_line.count;

					write(user_input_line);
					write('\n');

					execute();

					user_input_line.count = 0;
					history_index = -1;
					caret = 0;
				} else if (scancode == SDL_SCANCODE_UP) {
					history_index++;
					history_index = clamp(history_index, -1, (int)COMMAND_HISTORY - 1);

					if (history_index != -1) {
						memcpy(user_input_line_buf, command_history_buf[history_index], command_history[history_index].count);
						user_input_line.count = command_history[history_index].count;
						caret = user_input_line.count;
					} else {
						user_input_line.count = 0;
						caret = 0;
					}
				} else if (scancode == SDL_SCANCODE_DOWN) {
					history_index--;
					history_index = clamp(history_index, -1, (int)COMMAND_HISTORY - 1);

					if (history_index != -1) {
						memcpy(user_input_line_buf, command_history_buf[history_index], command_history[history_index].count);
						user_input_line.count = command_history[history_index].count;
						caret = user_input_line.count;
					} else {
						user_input_line.count = 0;
						caret = 0;
					}
				} else if (scancode == SDL_SCANCODE_RIGHT) {
					caret++;
					caret = clamp(caret, 0, (int)user_input_line.count);
				} else if (scancode == SDL_SCANCODE_LEFT) {
					caret--;
					caret = clamp(caret, 0, (int)user_input_line.count);
				}
			} else {
				if (scancode == SDL_SCANCODE_GRAVE) {
					show = true;
					user_input_line.count = 0;
					scroll = -2;
					history_index = -1;
					caret = 0;
				}
			}
			break;
		}

		case SDL_TEXTINPUT: {
			char ch = ev->text.text[0];

			if (show) {
				if (ch != '`' && ch >= 32 && ch <= 127) {
					array_insert(&user_input_line, caret, ch);
					caret++;
					caret = clamp(caret, 0, (int)user_input_line.count);
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
	string str = user_input_line;

	eat_whitespace(&str);
	string command = eat_non_whitespace(&str);

	if (command == "h" || command == "help") {
		string s = R"(Commands:
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
		string stage_index_str = eat_non_whitespace(&str);

		bool done;
		u32 stage_index = string_to_u32(stage_index_str, &done);

		if (done && stage_index < STAGE_COUNT) {
			g->stage_index     = stage_index;
			g->skip_to_boss    = false;
			g->skip_to_midboss = false;
			g->next_state      = Game::STATE_PLAYING;

			eat_whitespace(&str);
			string token = eat_non_whitespace(&str);

			if (token == "boss") {
				g->skip_to_boss = true;
			} else if (token == "midboss") {
				g->skip_to_midboss = true;
			}
		} else {
			write("invalid stage index\n");
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

	write("unknown command \"");
	write(command);
	write("\"\n");
}

void Console::write(char ch) {
	if (history.count == history.capacity) {
		array_remove(&history, history.begin());
	}

	array_add(&history, ch);
}

void Console::write(string str) {
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

	Sprite* font = GetSprite(spr_font_main);

	// Draw text
	r->draw_text(font, history, x, y, HALIGN_LEFT, VALIGN_BOTTOM);

	// Draw shell thing
	r->draw_text(font, ">", x, y, HALIGN_LEFT, VALIGN_BOTTOM);

	x += 16;

	// Draw current cmd
	r->draw_text(font, user_input_line, x, y, HALIGN_LEFT, VALIGN_BOTTOM);

	// Draw caret
	{
		vec4 color;
		if ((SDL_GetTicks() % 800) > 400) {
			color = {1, 1, 1, 1.00f};
		} else {
			color = {1, 1, 1, 0.50f};
		}

		string str;
		str.data  = user_input_line.data;
		str.count = caret;

		vec2 text_size = r->measure_text(font, str);

		r->draw_rectangle({(int)(x + text_size.x), (int)(y - text_size.y), 2, 16}, color);
	}

	r->break_batch();
	glDisable(GL_SCISSOR_TEST);
}
