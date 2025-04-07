#include "game.h"

#include "window_creation.h"
#include "package.h"
#include "console.h"
#include "title_screen.h"
#include "world.h"
#include "util.h"

#include <qoi/qoi.h>

#include <inttypes.h> // for PRIX64
#include <string.h> // for memset

Game game;

void Game::init() {
	stage_bg_fbo = load_framebuffer(PLAY_AREA_W, PLAY_AREA_H, GL_NEAREST, GL_CLAMP_TO_EDGE, GL_RGB, true);

	state = STATE_TITLE_SCREEN;
	title_screen.init();

	play_music("music/dbu_the_foolish_girl.mp3");
}

void Game::deinit() {
	switch (state) {
		case STATE_TITLE_SCREEN: title_screen.deinit(); break;
		case STATE_PLAYING:      world.deinit();        break;
	}

	free_framebuffer(&stage_bg_fbo);
}

void Game::update(float delta) {
	double time = get_time();

#ifdef DEVELOPER
	if (is_key_pressed(SDL_SCANCODE_F1)) {
		show_debug_info ^= true;
	}

	if (is_key_held(SDL_SCANCODE_F)) {
		delta *= 2.0f;
	}

	if (is_key_held(SDL_SCANCODE_S)) {
		delta *= 0.5f;
	}
#endif

	if (!window.should_skip_frame || state == STATE_PLAYING) {
		switch (state) {
			case STATE_TITLE_SCREEN: title_screen.update(delta); break;
			case STATE_PLAYING:      world.update(delta);        break;
		}
	}

	if (next_state != STATE_NONE) {
		switch (state) {
			case STATE_TITLE_SCREEN: title_screen.deinit(); break;
			case STATE_PLAYING:      world.deinit();        break;
		}

		state      = next_state;
		next_state = STATE_NONE;

		switch (state) {
			case STATE_TITLE_SCREEN:
				title_screen = {};
				title_screen.init();
				break;

			case STATE_PLAYING:
				world = {};
				world.init();
				break;
		}
	}
}

void Game::draw(float delta) {
	switch (state) {
		case STATE_TITLE_SCREEN: {
			title_screen.draw(delta);
			break;
		}

		case STATE_PLAYING: {
			world.draw(delta);
			break;
		}
	}

	// draw fps
	{
		char buf[10];
		string str = Sprintf(buf, "%.0ffps", roundf(window.avg_fps));
		draw_text(get_font(fnt_main), str, {35 * 16, 29 * 16});
	}
}


void Game::late_draw(float delta) {
	auto world_to_screen_x = [&](float x) {
		x += PLAY_AREA_X;
		return (x / GAME_W) * renderer.game_texture_rect.w + renderer.game_texture_rect.x;
	};

	auto world_to_screen_y = [&](float y) {
		y += PLAY_AREA_Y;
		return (y / GAME_H) * renderer.game_texture_rect.h + renderer.game_texture_rect.y;
	};

	vec2 pos = {};

	if (show_debug_info) {
#ifdef _DEBUG
		int debug_build = 1;
#else
		int debug_build = 0;
#endif

		char buf[256];
		string str = Sprintf(buf,
							"fps: %.2f\n"
							"update: %.2fms\n"
							"draw: %.2fms\n"
							"draw calls: %d\n"
							"max batch: %zu / %zu\n"
							"debug build: %d\n",
							window.fps,
							(window.frame_took - renderer.draw_took) * 1000.0,
							renderer.draw_took * 1000.0,
							renderer.draw_calls,
							renderer.max_batch, BATCH_MAX_VERTICES,
							debug_build);
		pos = draw_text(get_font(fnt_consolas_bold), str, pos);
		pos.y += 8;

		if (state == STATE_PLAYING) {
			char buf[256];
			string str = Sprintf(buf,
								"bullets: %zu / %zu\n"
								"enemies: %zu / %zu\n"
								"player bullets: %zu / %zu\n"
								"pickups: %zu / %zu\n"
								"next instance id: %" PRIX64 " / %" PRIX64 "\n"
								"coroutine memory: " Size_Fmt "\n"
								"animations: %zu / %zu\n"
								"temp arena for boss: " Size_Fmt " / " Size_Fmt "\n",
								world.bullets.count,   world.bullets.capacity,
								world.enemies.count,   world.enemies.capacity,
								world.p_bullets.count, world.p_bullets.capacity,
								world.pickups.count,   world.pickups.capacity,
								world.next_instance_id, (u64)UINT32_MAX,
								Size_Arg(world.coro_memory),
								world.animations.count, world.animations.capacity,
								Size_Arg(world.temp_arena_for_boss.count), Size_Arg(world.temp_arena_for_boss.capacity));
			pos = draw_text(get_font(fnt_consolas_bold), str, pos);
			pos.y += 8;

#define Object_Fmt "id: %" PRIX64 "\n"

#define Object_Arg(o) (o)->id

			{
				Player* p = &world.player;

				char buf[256];
				string str = Sprintf(buf,
									Object_Fmt
									"state: %s\n"
									"focused: %d\n"
									"iframes: %f\n"
									"timer: %f\n"
									"bomb_timer: %f\n"
									"lazer_graze_timer: %f\n",
									Object_Arg(p),
									GetPlayerStateName(p->state),
									p->focused,
									p->iframes,
									p->timer,
									p->bomb_timer,
									p->lazer_graze_timer);
				draw_text(get_font(fnt_consolas_bold), str, {world_to_screen_x(p->x), world_to_screen_y(p->y)});
			}

			if (!(world.boss.flags & FLAG_INSTANCE_DEAD)) {
				Boss* b = &world.boss;

				char buf[256];
				string str = Sprintf(buf,
									Object_Fmt
									"boss_index: %s\n"
									"state: %s\n"
									"phase_index: %d\n"
									"hp: %f\n"
									"timer: %f\n"
									"wait_timer: %f\n",
									Object_Arg(b),
									GetBossIndexName((BossIndex)b->boss_index),
									GetBossStateName(b->state),
									b->phase_index,
									b->hp,
									b->timer,
									b->wait_timer);
				draw_text(get_font(fnt_consolas_bold), str, {world_to_screen_x(b->x), world_to_screen_y(b->y)});
			}
		}
	}

	if (window.frame_advance_mode) {
		string str = "F5 - Next Frame\nF6 - Disable Frame Advance Mode\n";
		pos = draw_text(get_font(fnt_consolas_bold), str, pos);
	}
}




#ifdef DEVELOPER
static string s_ConsoleCommandsBuf[] = {
	"help",
	"stage",
	"title",
	"skip",
	"full_power",
	"life",
	"kill_player",
	"reload_shaders",
};

array<string> g_ConsoleCommands = s_ConsoleCommandsBuf;

bool console_callback(string str, void* userdata) {
	eat_whitespace(&str);
	string command = eat_non_whitespace(&str);

	if (command == "h" || command == "help") {
		console.write("Commands: ");
		For (it, g_ConsoleCommands) {
			console.write(*it);
			console.write(' ');
		}
		console.write("\n");

		return true;
	}

	if (command == "stage") {
		eat_whitespace(&str);
		string stage_index_str = eat_non_whitespace(&str);

		bool done;
		u32 stage_index = string_to_u32(stage_index_str, &done);

		if (done && stage_index < STAGE_COUNT) {
			game.stage_index     = stage_index;
			game.skip_to_boss    = false;
			game.skip_to_midboss = false;
			game.next_state      = Game::STATE_PLAYING;

			eat_whitespace(&str);
			string token = eat_non_whitespace(&str);

			if (token == "boss") {
				game.skip_to_boss = true;
			} else if (token == "midboss") {
				game.skip_to_midboss = true;
			}
		} else {
			console.write("invalid stage index\n");
		}
		return true;
	}

	if (command == "title") {
		game.next_state = Game::STATE_TITLE_SCREEN;
		return true;
	}

	if (command == "reload_shaders") {
		reload_shaders();
		return true;
	}

	if (game.state == Game::STATE_PLAYING) {
		if (command == "skip") {
			if (!(world.boss.flags & FLAG_INSTANCE_DEAD)) {
				world.boss.timer = 0;
				world.boss.wait_timer = 0;
			}
			return true;
		}

		if (command == "full_power") {
			get_power(MAX_POWER);
			return true;
		}

		if (command == "life") {
			get_lives(1);
			return true;
		}

		if (command == "kill_player") {
			player_get_hit(&world.player);
			return true;
		}
	}

	return false;
}
#endif
