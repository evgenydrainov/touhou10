#include "title_screen.h"

#include "game.h"
#include <glad/gl.h>

#define MENU_SIZE 3

void Title_Screen::init() {}

void Title_Screen::destroy() {}

void Title_Screen::update(float delta) {
	if (is_key_pressed(SDL_SCANCODE_UP)) {
		cursor--;
	}

	if (is_key_pressed(SDL_SCANCODE_DOWN)) {
		cursor++;
	}

	cursor = wrap(cursor, MENU_SIZE);

	if (is_key_pressed(SDL_SCANCODE_Z)) {
		switch (cursor) {
			case 0:
				g->stage_index     = 0;
				g->skip_to_boss    = false;
				g->skip_to_midboss = false;
				g->next_state      = Game::STATE_PLAYING;
				break;
		}
	}
}

void Title_Screen::draw(float delta) {
	glViewport(g->game_viewport.x, g->game_viewport.y, g->game_viewport.w, g->game_viewport.h);
	r->proj = glm::ortho<float>(0, GAME_W, GAME_H, 0);

	r->draw_text(GetSprite(spr_font_main), "The Title Screen.", 16, 16);

	string labels[MENU_SIZE] = {"Play", "Options", "Quit"};

	for (int i = 0; i < ArrayLength(labels); i++) {
		float x = GAME_W * 0.75f - 20.0f * i;
		float y = GAME_H * 0.55f + 50.0f * i;

		vec4 color = color_white;
		if (i == cursor) {
			color = color_yellow;
		}

		r->draw_text(GetSprite(spr_font_main), labels[i], x, y, HALIGN_LEFT, VALIGN_TOP, color);
	}

	r->break_batch();
}
