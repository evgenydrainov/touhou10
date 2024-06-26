#include "game.h"

#include "cpml.h"

#include <glad/gl.h>
#include <glm/gtc/matrix_transform.hpp>
#include <stb/stb_image.h>
#include <stb/stb_sprintf.h>
#include <qoi/qoi.h>

#include <inttypes.h>

Game* g;

static GLADapiproc glad_load_func(const char* name) {
	return (GLADapiproc) SDL_GL_GetProcAddress(name);
}

// @Copy
void _Assertion_Failed(const char* file, int line, const char* condition) {
	char buf[256];
	stb_snprintf(buf, sizeof(buf), "%s:%d:\n%s", file, line, condition);
	SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Assertion failed", buf, g->window);
	exit(1);
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


static_assert(NUM_SOUNDS == 16, "");
static String sounds_filenames[] = {
	"sounds/boss_die.wav",
	"sounds/char_reimu_shoot.wav",
	"sounds/enemy_die.wav",
	"sounds/enemy_hurt.wav",
	"sounds/enemy_shoot.wav",
	"sounds/extend.wav",
	"sounds/graze.wav",
	"sounds/lazer.wav",
	"sounds/menu_cancel.wav",
	"sounds/menu_navigate.wav",
	"sounds/menu_ok.wav",
	"sounds/pause.wav",
	"sounds/pichuun.wav",
	"sounds/pickup.wav",
	"sounds/powerup.wav",
	"sounds/spellcard.wav",
};


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


	// Create game framebuffer
	{
		glGenTextures(1, &game_texture);

		glBindTexture(GL_TEXTURE_2D, game_texture);
		Defer { glBindTexture(GL_TEXTURE_2D, 0); };

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); // GL_LINEAR for sharp bilinear shader
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
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

	// Setup arena
	{
		const size_t memory_for_package =
			Package::MAX_ENTRIES * sizeof(Package::Entry)
			+ Package::MEMORY_FOR_FILENAMES;

		const size_t memory_for_renderer = BATCH_MAX_VERTICES * sizeof(Vertex);

		const size_t memory_for_world =
			MAX_BULLETS          * sizeof(Bullet)
			+ MAX_PLAYER_BULLETS * sizeof(PlayerBullet)
			+ MAX_ENEMIES        * sizeof(Enemy)
			+ MAX_PICKUPS        * sizeof(Pickup)
			+ MAX_ANIMATIONS     * sizeof(Animation)
			+ MAX_PARTICLES      * sizeof(Particle)
			+ TEMP_STORAGE_FOR_BOSS;

		const size_t all_memory =
			memory_for_package
			+ memory_for_renderer
			+ memory_for_world
			+ Kilobytes(10); // Add some for alignment

		arena = ArenaAlloc(all_memory);

		// frame_arena = ArenaAlloc(1000);
	}

	Mix_Init(MIX_INIT_MP3);

	Mix_OpenAudio(MIX_DEFAULT_FREQUENCY, MIX_DEFAULT_FORMAT, MIX_DEFAULT_CHANNELS, 2048);

	{
		float master_volume = 0.50f;
		float sound_volume  = 0.50f;
		float music_volume  = 0.75f;

		Mix_Volume(-1,  (int)(MIX_MAX_VOLUME * (master_volume * sound_volume)));
		Mix_VolumeMusic((int)(MIX_MAX_VOLUME * (master_volume * music_volume)));
	}

	double loading_time = GetTime();

	package.init(&arena);
	package.load("package.dat");

	log_info("Read package header in %fms.", (GetTime() - loading_time) * 1000.0);
	loading_time = GetTime();

	// load textures
	{
		static_assert(NUM_TEXTURES == 8, "");

		// @Leak
		texture_data[tex_atlas_0]                     = load_texture("textures/atlas_0.png",                    true);
		texture_data[tex_stage_0_bg]                  = load_texture("textures/stage_0_bg.png",                 true);
		texture_data[tex_cirno_spellcard_background]  = load_texture("textures/cirno_spellcard_background.png", true);
		texture_data[tex_background]                  = load_texture("textures/background.png");
		texture_data[tex_white]                       = load_texture("textures/white.png");
		texture_data[tex_boss_cirno_portrait]         = load_texture("textures/boss_cirno_portrait.png");
		texture_data[tex_boss_youmu_portrait]         = load_texture("textures/boss_youmu_portrait.png");
		texture_data[tex_spellcard_attack_anim_label] = load_texture("textures/spellcard_attack_anim_label.png");
	}

	log_info("Loaded textures in %fms.", (GetTime() - loading_time) * 1000.0);
	loading_time = GetTime();

	// load sounds
	{

		static_assert(NUM_SOUNDS == 16, "");

		// @Leak
		for (int i = 0; i < NUM_SOUNDS; i++) {
			sound_data[i] = load_sound(sounds_filenames[i]);
		}

		Mix_VolumeChunk(sound_data[snd_enemy_shoot], (int)(MIX_MAX_VOLUME * 0.50f));
	}

	log_info("Loaded sounds in %fms.", (GetTime() - loading_time) * 1000.0);

	package.close();

	r = &renderer;
	r->init();

	// 
	// Arena will be reset to this position when game state changes.
	// 
	arena_pos_save = arena.count;

	w = &world;

	state = STATE_TITLE_SCREEN;
	title_screen.init();

	music = Mix_LoadMUS("music/dbu_the_foolish_girl.mp3");
	Mix_PlayMusic(music, -1);
}

void Game::destroy() {

	if (music) {
		Mix_FreeMusic(music);
		music = nullptr;
	}

	switch (state) {
		case STATE_TITLE_SCREEN: title_screen.destroy(); break;
		case STATE_PLAYING:      w->destroy();           break;
	}

	r->destroy();

	package.destroy();

	Mix_CloseAudio();
	Mix_Quit();

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

		// @Temp Don't draw if minimized?
		if (!(SDL_GetWindowFlags(window) & SDL_WINDOW_MINIMIZED)) {
			draw(delta);
		}

		// 
		// If you don't swap the window when window is minimized,
		// then you won't be able to alt-tab from it in fullscreen.
		// 
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
		switch (state) {
			case STATE_TITLE_SCREEN: title_screen.update(delta); break;
			case STATE_PLAYING:      w->update(delta);           break;
		}
	}

	console.update(delta);

	if (next_state != STATE_NONE) {
		switch (state) {
			case STATE_TITLE_SCREEN: title_screen.destroy(); break;
			case STATE_PLAYING:      w->destroy();           break;
		}

		state      = next_state;
		next_state = STATE_NONE;

		// Restore arena position
		arena.count = arena_pos_save;

		switch (state) {
			case STATE_TITLE_SCREEN:
				title_screen = {};
				title_screen.init();
				break;

			case STATE_PLAYING:
				*w = {};
				w->init();
				break;
		}
	}

	update_took = GetTime() - time;
}

void Game::draw(float delta) {
	double time = GetTime();

	int    draw_calls = r->draw_calls;
	size_t max_batch  = r->max_batch;

	r->draw_calls = 0;
	r->max_batch  = 0;

	// 
	// Render the game to a framebuffer.
	// 
	{
		glBindFramebuffer(GL_FRAMEBUFFER, game_fbo);
		Defer { glBindFramebuffer(GL_FRAMEBUFFER, 0); };

		glClearColor(0, 0, 0, 1);
		glClear(GL_COLOR_BUFFER_BIT);

		switch (state) {
			case STATE_TITLE_SCREEN: title_screen.draw(delta); break;
			case STATE_PLAYING:
				w->draw(delta); // Projection matrix will change after this call
				break;
		}
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
			u32 program = r->shader_sharp_bilinear_program;
			u32 old_texture_shader = r->current_texture_shader;

			r->current_texture_shader = program;
			glUseProgram(program);

			Texture t = {};
			t.ID = game_texture;
			t.width  = GAME_W;
			t.height = GAME_H;

			{
				int u_source_size = glGetUniformLocation(program, "u_SourceSize");
				glUniform2f(u_source_size, (float)t.width, (float)t.height);
			}

			{
				float int_scale = max(floorf(scale), 1.0f);

				int u_scale = glGetUniformLocation(program, "u_Scale");
				glUniform2f(u_scale, int_scale, int_scale);
			}

			r->draw_texture(&t, {}, {game_texture_x, game_texture_y}, {scale, scale}, {}, 0, color_white, {false, true});

			r->break_batch();

			r->current_texture_shader = old_texture_shader;
		}

		auto world_to_screen_x = [&](float x) {
			return (x + (float)PLAY_AREA_X) * scale + game_texture_x;
		};

		auto world_to_screen_y = [&](float y) {
			return (y + (float)PLAY_AREA_Y) * scale + game_texture_y;
		};

		if (show_debug_info) {
			Static_String<256> buf;
			Sprintf(&buf,
					"fps: %.2f\n"
					"update: %.2fms\n"
					"draw: %.2fms\n"
					"arena: " Size_Fmt " / " Size_Fmt "\n"
					"frame arena: " Size_Fmt " / " Size_Fmt "\n"
					"draw calls: %d\n"
					"max batch: %zu / %zu\n"
#if TH_DEBUG
					"COMPILED WITH DEBUG\n"
#endif
					"__cplusplus=%ld\n",
					fps,
					update_took * 1000.0,
					draw_took * 1000.0,
					Size_Arg(arena.count),       Size_Arg(arena.capacity),
					Size_Arg(frame_arena.count), Size_Arg(frame_arena.capacity),
					draw_calls,
					max_batch, BATCH_MAX_VERTICES,
					__cplusplus);
			glm::vec2 pos = r->draw_text(GetSprite(spr_font_main), buf, 0, 0);
			pos.y += 8;

			// Audio Debug
			{
				auto find_chunk_filename = [&](Mix_Chunk* chunk) -> String {
					for (int i = 0; i < NUM_SOUNDS; i++) {
						if (sound_data[i] == chunk) {
							return sounds_filenames[i];
						}
					}
					return "none";
				};

				int nchannels = Mix_AllocateChannels(-1);
				for (int i = 0; i < nchannels; i++) {
					Mix_Chunk* chunk = Mix_GetChunk(i);
					String filename = find_chunk_filename(chunk);

					Static_String<64> buf;
					Sprintf(&buf, "%d " Str_Fmt "\n", i, Str_Arg(filename));

					glm::vec4 color = Mix_Playing(i) ? color_white : glm::vec4{0.5f, 0.5f, 0.5f, 1};
					pos = r->draw_text(GetSprite(spr_font_main), buf, pos.x, pos.y, HALIGN_LEFT, VALIGN_TOP, color);
				}

				pos.y += 8;
			}

			if (state == STATE_PLAYING) {
				Static_String<256> buf;
				Sprintf(&buf,
						"bullets: %zu / %zu\n"
						"enemies: %zu / %zu\n"
						"player bullets: %zu / %zu\n"
						"pickups: %zu / %zu\n"
						"next instance id: %" PRIX64 " / %" PRIX64 "\n"
						"coroutine memory: " Size_Fmt "\n"
						"animations: %zu / %zu\n"
						"temp arena for boss: " Size_Fmt " / " Size_Fmt "\n",
						w->bullets.count,   w->bullets.capacity,
						w->enemies.count,   w->enemies.capacity,
						w->p_bullets.count, w->p_bullets.capacity,
						w->pickups.count,   w->pickups.capacity,
						w->next_instance_id, (u64)UINT32_MAX,
						Size_Arg(w->coro_memory),
						w->animations.count, w->animations.capacity,
						Size_Arg(w->temp_arena_for_boss.count), Size_Arg(w->temp_arena_for_boss.capacity));
				pos = r->draw_text(GetSprite(spr_font_main), buf, pos.x, pos.y);
				pos.y += 8;

#define Object_Fmt "id: %" PRIX64 "\n"

#define Object_Arg(o) (o)->id

				{
					Player* p = &w->player;

					Static_String<256> buf;
					Sprintf(&buf,
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
					r->draw_text(GetSprite(spr_font_main), buf, world_to_screen_x(p->x), world_to_screen_y(p->y));
				}

				if (!(w->boss.flags & FLAG_INSTANCE_DEAD)) {

					Boss* b = &w->boss;

					Static_String<256> buf;
					Sprintf(&buf,
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

	Assert(false);

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

Texture load_texture(String fname, bool filter) {

	auto create_texture = [&](void* pixel_data, int width, int height, int num_channels) -> Texture {
		u32 texture;
		glGenTextures(1, &texture);

		glBindTexture(GL_TEXTURE_2D, texture);
		Defer { glBindTexture(GL_TEXTURE_2D, 0); };

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter ? GL_LINEAR : GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter ? GL_LINEAR : GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

		Assert(num_channels == 4);

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixel_data);

		return {texture, width, height};
	};

	Texture result = {};

	size_t filesize;
	u8* filedata = g->package.get_file(fname, &filesize);

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
		log_info("Loaded texture " Str_Fmt, Str_Arg(fname));
	} else {
		log_info("Couldn't load texture " Str_Fmt, Str_Arg(fname));

		// Create a 1x1 stub texture
		static u32 white[1 * 1] = {0xffffffff};

		result = create_texture(white, 1, 1, 4);
	}

	return result;
}

Mix_Chunk* load_sound(String fname) {
	Mix_Chunk* result = nullptr;

	size_t filesize;
	u8* filedata = g->package.get_file(fname, &filesize);

	if (filedata) {
		// Goddammit SDL.
		Assert(filesize <= INT_MAX);
		SDL_RWops* src = SDL_RWFromConstMem(filedata, (int) filesize);
		result = Mix_LoadWAV_RW(src, 1);

		log_info("Loaded sound " Str_Fmt, Str_Arg(fname));
	} else {
		log_info("Couldn't load sound " Str_Fmt, Str_Arg(fname));
	}

	return result;
}

void stop_sound(u32 sound_index) {
	Mix_Chunk* chunk = GetSound(sound_index);

	int nchannels = Mix_AllocateChannels(-1);
	for (int i = 0; i < nchannels; i++) {
		if (Mix_Playing(i) && Mix_GetChunk(i) == chunk) {
			Mix_HaltChannel(i);
		}
	}
}

void play_sound(u32 sound_index) {
	stop_sound(sound_index);

	Mix_Chunk* chunk = GetSound(sound_index);
	Mix_PlayChannel(-1, chunk, 0);
}
