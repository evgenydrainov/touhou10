#include "common.h"
#include "window_creation.h"
#include "game.h"
#include "renderer.h"
#include "package.h"

#if defined(DEVELOPER)
#include "console.h"
#endif


int main(int /*argc*/, char* /*argv*/[]) {
	init_window_and_opengl("touhou10", 640, 480, 2, false, false);
	defer { deinit_window_and_opengl(); };

	init_package();
	defer { deinit_package(); };

	init_renderer();
	defer { deinit_renderer(); };

	game.init();
	defer { game.deinit(); };

#if defined(DEVELOPER)
	console.init(console_callback, nullptr, get_font(fnt_main));
	defer { console.deinit(); };
#endif

	while (!window.should_quit) {
		begin_frame();

		SDL_Event ev;
		while (SDL_PollEvent(&ev)) {
			handle_event(ev);

#if defined(DEVELOPER)
			console.handle_event(ev);
#endif
		}

		// update
		game.update(window.delta);

#if defined(DEVELOPER)
		console.update(window.delta);
#endif

		// render
		vec4 clear_color = color_black;
		render_begin_frame(clear_color);

		game.draw(window.delta);

		render_end_frame();

		game.late_draw(window.delta);

#if defined(DEVELOPER)
		console.draw(window.delta);
#endif

		swap_buffers();
	}

	return 0;
}
