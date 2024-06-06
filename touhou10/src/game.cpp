#include "game.h"

#include "cpml.h"

#include <glad/gl.h>
#include <glm/gtc/matrix_transform.hpp>
#include <stb/stb_image.h>
#include <stb/stb_sprintf.h>
#include <qoi/qoi.h>

Game* g;

static GLADapiproc glad_load_func(const char* name) {
	return (GLADapiproc) SDL_GL_GetProcAddress(name);
}

#if TH_DEBUG
static void GLAPIENTRY glDebugOutput(GLenum source,
									 GLenum type,
									 unsigned int id,
									 GLenum severity,
									 GLsizei length,
									 const char *message,
									 const void *userParam);
#endif

void Game::init() {
	SDL_LogSetAllPriority(SDL_LOG_PRIORITY_VERBOSE);

	SDL_SetHint("SDL_WINDOWS_DPI_AWARENESS", "system");

	SDL_Init(SDL_INIT_VIDEO
			 | SDL_INIT_AUDIO);

	window = SDL_CreateWindow("touhou10",
							  SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
							  2 * GAME_W, 2 * GAME_H,
							  SDL_WINDOW_OPENGL
							  | SDL_WINDOW_RESIZABLE);

	SDL_SetWindowMinimumSize(window, GAME_W, GAME_H);

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

#if TH_DEBUG
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);
#endif

	gl_context = SDL_GL_CreateContext(window);
	SDL_GL_MakeCurrent(window, gl_context);

	{
		int version = gladLoadGL(glad_load_func);
		log_info("Loaded GL %d.%d", GLAD_VERSION_MAJOR(version), GLAD_VERSION_MINOR(version));
	}

#if TH_DEBUG
	if (GLAD_GL_ARB_debug_output) {
		glDebugMessageCallbackARB(glDebugOutput, nullptr);
	}
#endif

	/*
	{
		int num_exts;
		glGetIntegerv(GL_NUM_EXTENSIONS, &num_exts);

		log_info("%d extensions:", num_exts);

		for (int i = 0; i < num_exts; i++) {
			const char* ext = (const char*) glGetStringi(GL_EXTENSIONS, i);
			log_info("%s", ext);
		}
	}
	*/

	SDL_GL_SetSwapInterval(0);

	glDisable(GL_CULL_FACE);
	// glEnable(GL_CULL_FACE);
	// glCullFace(GL_FRONT);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	// glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);


	{
		glGenTextures(1, &game_texture);

		glBindTexture(GL_TEXTURE_2D, game_texture);
		Defer { glBindTexture(GL_TEXTURE_2D, 0); };

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, GAME_W, GAME_H, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
	}
	
	{
		glGenFramebuffers(1, &game_fbo);

		glBindFramebuffer(GL_FRAMEBUFFER, game_fbo);
		Defer { glBindFramebuffer(GL_FRAMEBUFFER, 0); };

		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, game_texture, 0);
	}

	{
		static_assert(NUM_TEXTURES == 4, "");

		texture_data[tex_atlas_0]                    = load_texture("textures/atlas_0.qoi");
		texture_data[tex_stage_0_bg]                 = load_texture("textures/stage_0_bg.qoi",                 true);
		texture_data[tex_cirno_spellcard_background] = load_texture("textures/cirno_spellcard_background.qoi", true);
		texture_data[tex_background]                 = load_texture("textures/background.qoi");
	}

	{
		const size_t memory_for_renderer = BATCH_MAX_VERTICES * sizeof(Vertex);

		const size_t memory_for_world =
			MAX_BULLETS * sizeof(Bullet)
			+ MAX_PLAYER_BULLETS * sizeof(PlayerBullet);

		const size_t all_memory =
			memory_for_renderer
			+ memory_for_world
			+ Kilobytes(10); // Add some for alignment

		arena = ArenaAlloc(all_memory);
	}

	// frame_arena = ArenaAlloc(1000);

	r = &renderer;
	r->init();

	w = &world;
	w->init();

}

void Game::destroy() {
	w->destroy();

	r->destroy();

	ArenaRelease(&frame_arena);
	ArenaRelease(&arena);

	SDL_GL_DeleteContext(gl_context);

	SDL_DestroyWindow(window);

	SDL_Quit();
}

double GetTime() {
	return (double)SDL_GetPerformanceCounter() / (double)SDL_GetPerformanceFrequency();
}

void Game::run() {

	double prev_time = GetTime() - (1.0 / 60.0);

	bool quit = false;
	while (!quit) {

		double time = GetTime();
		double frame_end_time = time + (1.0 / 60.0);
		fps = 1.0 / (time - prev_time);
		prev_time = time;

		memset(key_pressed, 0, sizeof(key_pressed));
		memset(key_repeat, 0, sizeof(key_repeat));

		skip_frame = frame_advance;

		{
			SDL_Event ev;
			while (SDL_PollEvent(&ev)) {
				switch (ev.type) {
					case SDL_QUIT: {
						quit = true;
						break;
					}

					case SDL_KEYDOWN: {
						SDL_Scancode scancode = ev.key.keysym.scancode;

						switch (scancode) {
							case SDL_SCANCODE_F1: {
								show_debug_info ^= true;
								break;
							}

							case SDL_SCANCODE_F4: {
								if (ev.key.keysym.mod != 0) {
									break;
								}
								if (SDL_GetWindowFlags(window) & SDL_WINDOW_FULLSCREEN) {
									SDL_SetWindowFullscreen(window, 0);
								} else {
									SDL_DisplayMode mode;
									int display = SDL_GetWindowDisplayIndex(window);
									SDL_GetDesktopDisplayMode(display, &mode);
									SDL_SetWindowDisplayMode(window, &mode);
									SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN);
								}
								break;
							}

							case SDL_SCANCODE_F5: {
								frame_advance = true;
								skip_frame = false;
								break;
							}

							case SDL_SCANCODE_F6: {
								frame_advance = false;
								break;
							}
						}

						if (scancode < SDL_SCANCODE_UP + 1) {
							if (ev.key.repeat) {
								key_repeat[scancode / 32] |= 1 << (scancode % 32);
							} else {
								key_pressed[scancode / 32] |= 1 << (scancode % 32);
							}
						}

						break;
					}
				}

				console.event(&ev);
			}
		}

		float delta = 1;

		update(delta);

		draw(delta);

		SDL_GL_SwapWindow(window);

		double time_left = frame_end_time - GetTime();
		if (time_left > 0.0) {
			double time_to_sleep = time_left * 0.95;
			SDL_Delay((unsigned int)(time_to_sleep * 1000.0));

			while (GetTime() < frame_end_time) {}
		}
	}

}

void Game::update(float delta) {
	double time = GetTime();

	if (!skip_frame) {
		w->update(delta);
	}

	console.update(delta);

	update_took = GetTime() - time;
}

void Game::draw(float delta) {

	double time = GetTime();

	int draw_calls = r->draw_calls;
	size_t max_batch = r->max_batch;

	r->draw_calls = 0;
	r->max_batch = 0;

	// 
	// Render the game to a framebuffer.
	// 
	{
		glBindFramebuffer(GL_FRAMEBUFFER, game_fbo);
		Defer { glBindFramebuffer(GL_FRAMEBUFFER, 0); };

		glClearColor(0, 0, 0, 1);
		glClear(GL_COLOR_BUFFER_BIT);

		// Projection matrix will change after this call
		w->draw(delta);
	}

	int backbuffer_width;
	int backbuffer_height;
	SDL_GL_GetDrawableSize(window, &backbuffer_width, &backbuffer_height);

	glViewport(0, 0, backbuffer_width, backbuffer_height);
	r->proj = glm::ortho(0.0f, (float)backbuffer_width, (float)backbuffer_height, 0.0f);

	glClearColor(0, 0, 0, 1);
	glClear(GL_COLOR_BUFFER_BIT);

	// 
	// Render the framebuffer to screen.
	// 
	{
		float xscale = backbuffer_width  / (float)GAME_W;
		float yscale = backbuffer_height / (float)GAME_H;
		float scale = min(xscale, yscale);

		float game_texture_x = floorf((backbuffer_width  - GAME_W * scale) / 2.0f);
		float game_texture_y = floorf((backbuffer_height - GAME_H * scale) / 2.0f);

		{
			Texture t = {};
			t.ID = game_texture;
			t.width  = GAME_W;
			t.height = GAME_H;
			r->draw_texture(&t, {}, {game_texture_x, game_texture_y}, {scale, scale}, {}, 0, color_white, {false, true});
		}

		auto world_to_screen_x = [&](float x) {
			return (x + (float)PLAY_AREA_X) * scale + game_texture_x;
		};

		auto world_to_screen_y = [&](float y) {
			return (y + (float)PLAY_AREA_Y) * scale + game_texture_y;
		};

		if (show_debug_info) {
			char buf[256];
			stb_snprintf(buf, sizeof(buf),
						 "fps: %.2f\n"
						 "update: %.2fms\n"
						 "draw: %.2fms\n"
						 "arena: " Size_Fmt " / " Size_Fmt "\n"
						 "frame arena: " Size_Fmt " / " Size_Fmt "\n"
						 "draw calls: %d\n"
						 "max batch: %zu / %zu\n",
						 fps,
						 update_took * 1000.0,
						 draw_took * 1000.0,
						 Size_Arg(arena.offset), Size_Arg(arena.size),
						 Size_Arg(frame_arena.offset), Size_Arg(frame_arena.size),
						 draw_calls,
						 max_batch, BATCH_MAX_VERTICES);
			glm::vec2 pos = r->draw_text(GetSprite(spr_font_main), buf, 0, 0);

			/*if (in play state)*/ {
				char buf[256];
				stb_snprintf(buf, sizeof(buf),
							 "bullets: %zu / %zu\n"
							 "player bullets: %zu / %zu\n"
							 "next instance id: %llX / %llX\n"
							 "coroutine memory: " Size_Fmt "\n",
							 w->bullets.count, w->bullets.capacity,
							 w->p_bullets.count, w->p_bullets.capacity,
							 w->next_instance_id, (u64)UINT32_MAX,
							 Size_Arg(w->coro_memory));
				r->draw_text(GetSprite(spr_font_main), buf, pos.x, pos.y);

#define Object_Fmt "id: %llX\n"

#define Object_Arg(o) (o)->id

				{
					Player* p = &w->player;

					char buf[256];
					stb_snprintf(buf, sizeof(buf),
								 Object_Fmt
								 "focused: %d\n",
								 Object_Arg(p),
								 p->focused);
					r->draw_text(GetSprite(spr_font_main), buf, world_to_screen_x(p->x), world_to_screen_y(p->y));
				}

				if (!(w->boss.flags & FLAG_INSTANCE_DEAD)) {

					Boss* b = &w->boss;

					char buf[256];
					stb_snprintf(buf, sizeof(buf),
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
					r->draw_text(GetSprite(spr_font_main), buf, world_to_screen_x(b->x), world_to_screen_y(b->y));
				}
			}

			
		}

		console.draw(delta);

		r->break_batch();
	}

	draw_took = GetTime() - time;
}

#if TH_DEBUG
static void GLAPIENTRY glDebugOutput(GLenum source,
									 GLenum type,
									 unsigned int id,
									 GLenum severity,
									 GLsizei length,
									 const char *message,
									 const void *userParam) {
	// ignore non-significant error/warning codes
	// if (id == 131169 || id == 131185 || id == 131218 || id == 131204) {
	// 	return;
	// }

	log_info("---------------");
	log_info("Debug message (%u):\n%s", id, message);

	// SDL_Window* win = SDL_GL_GetCurrentWindow();
	// SDL_ShowSimpleMessageBox(0, "", message, win);

	__debugbreak();

	/*
	switch (source)
	{
		case GL_DEBUG_SOURCE_API:             log_info("Source: API"); break;
		case GL_DEBUG_SOURCE_WINDOW_SYSTEM:   log_info("Source: Window System"); break;
		case GL_DEBUG_SOURCE_SHADER_COMPILER: log_info("Source: Shader Compiler"); break;
		case GL_DEBUG_SOURCE_THIRD_PARTY:     log_info("Source: Third Party"); break;
		case GL_DEBUG_SOURCE_APPLICATION:     log_info("Source: Application"); break;
		case GL_DEBUG_SOURCE_OTHER:           log_info("Source: Other"); break;
	}

	switch (type)
	{
		case GL_DEBUG_TYPE_ERROR:               log_info("Type: Error"); break;
		case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: log_info("Type: Deprecated Behaviour"); break;
		case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  log_info("Type: Undefined Behaviour"); break;
		case GL_DEBUG_TYPE_PORTABILITY:         log_info("Type: Portability"); break;
		case GL_DEBUG_TYPE_PERFORMANCE:         log_info("Type: Performance"); break;
		case GL_DEBUG_TYPE_MARKER:              log_info("Type: Marker"); break;
		case GL_DEBUG_TYPE_PUSH_GROUP:          log_info("Type: Push Group"); break;
		case GL_DEBUG_TYPE_POP_GROUP:           log_info("Type: Pop Group"); break;
		case GL_DEBUG_TYPE_OTHER:               log_info("Type: Other"); break;
	}

	switch (severity)
	{
		case GL_DEBUG_SEVERITY_HIGH:         log_info("Severity: high"); break;
		case GL_DEBUG_SEVERITY_MEDIUM:       log_info("Severity: medium"); break;
		case GL_DEBUG_SEVERITY_LOW:          log_info("Severity: low"); break;
		case GL_DEBUG_SEVERITY_NOTIFICATION: log_info("Severity: notification"); break;
	}
	*/
}
#endif

static u8* read_entire_file(const char* fname, size_t* out_size) {
	u8* result = nullptr;

	FILE* f = nullptr;
	fopen_s(&f, fname, "rb");
	Defer { if (f) fclose(f); };

	if (f) {
		fseek(f, 0, SEEK_END);
		size_t size = (size_t) ftell(f);

		result = (u8*) malloc(size);
		Assert(result);

		fseek(f, 0, SEEK_SET);
		fread(result, size, 1, f);

		*out_size = size;
	}

	return result;
}

static bool is_png(u8* filedata, size_t filesize) {
	static u8 magic[] = {137, 80, 78, 71, 13, 10, 26, 10};
	if (filesize < sizeof(magic)) {
		return false;
	}
	for (size_t i = 0; i < sizeof(magic); i++) {
		if (filedata[i] != magic[i]) {
			return false;
		}
	}
	return true;
}

static bool is_qoi(u8* filedata, size_t filesize) {
	static u8 magic[] = {'q', 'o', 'i', 'f'};
	if (filesize < sizeof(magic)) {
		return false;
	}
	for (size_t i = 0; i < sizeof(magic); i++) {
		if (filedata[i] != magic[i]) {
			return false;
		}
	}
	return true;
}

Texture load_texture(const char* fname, bool filter) {

	auto create_texture = [&](void* pixel_data, int width, int height, int num_channels) -> Texture {
		u32 texture;
		glGenTextures(1, &texture);

		glBindTexture(GL_TEXTURE_2D, texture);
		Defer { glBindTexture(GL_TEXTURE_2D, 0); };

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter ? GL_LINEAR : GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter ? GL_LINEAR : GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		Assert(num_channels == 4);

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixel_data);

		return {texture, width, height};
	};

	Texture result = {};

	size_t filesize;
	u8* filedata = read_entire_file(fname, &filesize);
	Defer { if (filedata) free(filedata); };

	if (filedata) {
		if (is_png(filedata, filesize)) {
			int width;
			int height;
			int num_channels;
			void* pixel_data = stbi_load_from_memory(filedata, (int)filesize, &width, &height, &num_channels, 4);
			Defer { if (pixel_data) stbi_image_free(pixel_data); };

			result = create_texture(pixel_data, width, height, num_channels);
		} else if (is_qoi(filedata, filesize)) {
			qoi_desc desc;
			void* pixel_data = qoi_decode(filedata, (int)filesize, &desc, 4);
			Defer { if (pixel_data) free(pixel_data); };

			// Assert(desc.colorspace == QOI_SRGB);

			result = create_texture(pixel_data, desc.width, desc.height, desc.channels);
		}
	}

	if (result.ID != 0) {
		log_info("Loaded texture %s", fname);
	} else {
		log_info("Couldn't load texture %s", fname);

		// Create a 1x1 stub texture
		static u32 white[1 * 1] = {0xffffffff};

		result = create_texture(white, 1, 1, 4);
	}

	return result;
}
