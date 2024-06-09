#include "console.h"

#include "game.h"
#include "cpml.h"
#include <glad/gl.h>

#include <string.h>

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
						cmd[caret - 1] = 0;
						caret--;
					}
				} else if (scancode == SDL_SCANCODE_RETURN) {
					static_assert(sizeof(prev_cmd) == sizeof(cmd), "");
					memcpy(prev_cmd, cmd, sizeof(cmd));
					prev_caret = caret;

					write(cmd, caret);
					write('\n');
					execute();
					cmd[0] = 0;
					caret = 0;
				} else if (scancode == SDL_SCANCODE_UP) {
					static_assert(sizeof(prev_cmd) == sizeof(cmd), "");
					memcpy(cmd, prev_cmd, sizeof(cmd));
					caret = prev_caret;
				} else if (scancode == SDL_SCANCODE_DOWN) {
					cmd[0] = 0;
					caret = 0;
				}
			} else {
				if (scancode == SDL_SCANCODE_GRAVE) {
					show = true;
					cmd[0] = 0;
					caret = 0;
					scroll = 0;
				}
			}
			break;
		}

		case SDL_TEXTINPUT: {
			char ch = ev->text.text[0];

			if (show) {
				if (ch != '`' && ch >= 32 && ch <= 127) {
					if (caret < ArrayLength(cmd) - 1) {
						cmd[caret] = ch;
						cmd[caret + 1] = 0;
						caret++;
					}
				}
			}
			break;
		}

		case SDL_MOUSEWHEEL: {
			if (show) {
				scroll += 5 * ev->wheel.preciseY;
			}
			break;
		}
	}
}

void Console::execute() {
	if ((cmd[0] == 'h' && cmd[1] == 0) || strcmp(cmd, "help") == 0) {
		char buf[] = "The fog is coming\n";
		write(buf, sizeof(buf) - 1);
	} else if (strcmp(cmd, "skip") == 0) {
		if (!(w->boss.flags & FLAG_INSTANCE_DEAD)) {
			w->boss.timer = 0;
			w->boss.wait_timer = 0;
		}
	} else if (strcmp(cmd, "full_power") == 0) {
		g->stats.power = MAX_POWER;
	} else if (strcmp(cmd, "life") == 0) {
		g->stats.lives++;
		g->stats.lives = min(g->stats.lives, 8);
	} else if (strcmp(cmd, "kill_player") == 0) {
		w->player.state = PLAYER_STATE_DYING;
		w->player.timer = PLAYER_DEATH_TIME;
		play_sound(snd_pichuun);
	}
}

void Console::write(char ch) {
	if (text_pos >= ArrayLength(text) - 1) {
		for (size_t i = 0; i < ArrayLength(text) - 1; i++) {
			text[i] = text[i + 1];
		}
		text_pos--;
	}

	text[text_pos] = ch;
	text[text_pos + 1] = 0;
	text_pos++;
}

void Console::write(const char* buf, size_t size) {
#if 0
	if (text_pos + size + 1 > ArrayLength(text)) {
		size_t need = text_pos + size + 1 - ArrayLength(text);

		if (need <= ArrayLength(text)) {
			size_t move = ArrayLength(text) - need;
			todo();
		}
	}

	if (text_pos + size + 1 <= ArrayLength(text)) {
		memcpy(text + text_pos, buf, size);
		text[text_pos + size] = 0;
		text_pos += size;
	}
#else
	for (size_t i = 0; i < size; i++) {
		write(buf[i]);
	}
#endif
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
	r->draw_text(GetSprite(spr_font_main), text, x, y, HALIGN_LEFT, VALIGN_BOTTOM);

	// Draw shell thing
	r->draw_text(GetSprite(spr_font_main), ">", x, y, HALIGN_LEFT, VALIGN_BOTTOM);

	x += 16;

	// Draw current cmd
	r->draw_text(GetSprite(spr_font_main), cmd, x, y, HALIGN_LEFT, VALIGN_BOTTOM);

	r->break_batch();
	glDisable(GL_SCISSOR_TEST);
}
