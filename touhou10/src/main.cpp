#include "common.h"
#include "window_creation.h"
#include "renderer.h"
#include "package.h"
#include "assets.h"
#include "sound_mixer.h"
#include "game.h"

#ifdef DEVELOPER
#include "console.h"
#endif


int main(int /*argc*/, char* /*argv*/[]) {
	init_window_and_opengl("touhou10", 640, 480, 2, false, false);
	defer { deinit_window_and_opengl(); };

	init_mixer();
	defer { deinit_mixer(); };

	init_package();
	defer { deinit_package(); };

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
		begin_frame();

		SDL_Event ev;
		while (SDL_PollEvent(&ev)) {
			handle_event(ev);

#ifdef DEVELOPER
			console.handle_event(ev);
#endif
		}

		// update
		game.update(window.delta);

#ifdef DEVELOPER
		console.update(window.delta);
#endif

		// render
		vec4 clear_color = color_black;
		render_begin_frame(clear_color);

		game.draw(window.delta);

		render_end_frame();

		game.late_draw(window.delta);

#ifdef DEVELOPER
		console.draw(window.delta);
#endif

		swap_buffers();
	}

	return 0;
}
