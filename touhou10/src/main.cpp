#include "common.h"
#include "window_creation.h"
#include "renderer.h"
#include "package.h"
#include "assets.h"
#include "sound_mixer.h"
#include "game.h"
#include "input.h"

#ifdef DEVELOPER
#include "console.h"
#endif


static void do_one_frame() {
	begin_frame();

	// handle events
	{
		input.clear();

		SDL_Event ev;
		while (SDL_PollEvent(&ev)) {
			bool handled = false;

			if (!handled) handled = handle_event(ev);

			#ifdef DEVELOPER
				if (!handled) handled = console.handle_event(ev);
			#endif

			if (!handled) handled = input.handle_event(ev);
		}
	}

	// update
	{
		input.update(window.delta);

		game.update(window.delta);

		#ifdef DEVELOPER
			console.update(window.delta);
		#endif
	}

	// draw
	{
		vec4 clear_color = {};
		render_begin_frame(clear_color);

		game.draw(window.delta);

		render_end_frame();
	}

	// late draw
	{
		game.late_draw(window.delta);

		#ifdef DEVELOPER
			console.draw(window.delta);
		#endif

		break_batch();
	}

	swap_buffers();
}


int main(int argc, char* argv[]) {
	init_window_and_opengl("touhou10", GAME_W, GAME_H, 2, false, false);
	defer { deinit_window_and_opengl(); };

	init_package();
	defer { deinit_package(); };

	input.init();
	defer { input.deinit(); };

	init_mixer();
	defer { deinit_mixer(); };

	load_global_assets();
	defer { free_all_assets(); };

	init_renderer();
	defer { deinit_renderer(); };

	game.init();
	defer { game.deinit(); };

	#ifdef DEVELOPER
		console.init(console_callback, nullptr, get_font(fnt_consolas_bold), g_ConsoleCommands);
		defer { console.deinit(); };
	#endif

	while (!window.should_quit) {
		do_one_frame();
	}

	return 0;
}
