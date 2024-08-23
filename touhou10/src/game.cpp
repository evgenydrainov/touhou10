#include "game.h"

#include <glad/gl.h>
#include <stb/stb_image.h>
#include <qoi/qoi.h>

#include <inttypes.h> // for PRIX64
#include <string.h> // for memset

Game* g;

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
	log_info("Debug message (%u): %s", id, message);

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

	// SDL_Window* win = SDL_GL_GetCurrentWindow();
	// SDL_ShowSimpleMessageBox(0, "", message, win);

	SDL_TriggerBreakpoint();
}
#endif


static_assert(NUM_SOUNDS == 17, "");
static string sounds_filenames[] = {
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
	"sounds/kira.wav",
};


void Game::init() {
	// SDL_LogSetAllPriority(SDL_LOG_PRIORITY_VERBOSE);

	SDL_SetHint("SDL_WINDOWS_DPI_AWARENESS", "system");

#if TH_DEBUG
	SDL_setenv("SDL_ASSERT", "break", 0);
#endif

	if (SDL_Init(SDL_INIT_VIDEO
				 | SDL_INIT_AUDIO) != 0) {
		panic_and_abort("Couldn't initialize SDL: %s", SDL_GetError());
	}

	{
		log_info("Platform: %s", SDL_GetPlatform());
	}

	{
		SDL_version ver;

		SDL_VERSION(&ver);
		log_info("Compiled against SDL %u.%u.%u", ver.major, ver.minor, ver.patch);

		SDL_GetVersion(&ver);
		log_info("Linked against SDL %u.%u.%u", ver.major, ver.minor, ver.patch);
	}

	{
		// printf instead of log_info here because I want them in one line
		printf("INFO: Available video backends: ");
		for (int i = 0, n = SDL_GetNumVideoDrivers(); i < n; i++) {
			printf("%s ", SDL_GetVideoDriver(i));
		}
		printf("\n");

		log_info("Current video backend: %s", SDL_GetCurrentVideoDriver());
	}

	{
		printf("INFO: Available audio backends: ");
		for (int i = 0, n = SDL_GetNumAudioDrivers(); i < n; i++) {
			printf("%s ", SDL_GetAudioDriver(i));
		}
		printf("\n");

		log_info("Current audio backend: %s", SDL_GetCurrentAudioDriver());
	}

#if 0
	{
		printf("INFO: Available render backends: ");
		for (int i = 0, n = SDL_GetNumRenderDrivers(); i < n; i++) {
			SDL_RendererInfo info;
			SDL_GetRenderDriverInfo(i, &info);
			printf("%s ", info.name);
		}
		printf("\n");
	}
#endif

	window = SDL_CreateWindow("touhou10",
							  SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
							  2 * GAME_W, 2 * GAME_H,
							  SDL_WINDOW_OPENGL
							  | SDL_WINDOW_RESIZABLE);

	if (!window) {
		panic_and_abort("Couldn't create window: %s", SDL_GetError());
	}


	// 
	// A little workaround for Linux Mint Cinnamon.
	// 
#if TH_DEBUG && defined(__unix__)
	SDL_RaiseWindow(window);
#endif


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
		int version = gladLoadGL([](const char* name) {
			return (GLADapiproc) SDL_GL_GetProcAddress(name);
		});

		log_info("Loaded GL %d.%d", GLAD_VERSION_MAJOR(version), GLAD_VERSION_MINOR(version));
	}


#if TH_DEBUG
	if (GLAD_GL_KHR_debug) {
		glEnable(GL_DEBUG_OUTPUT);
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
		glDebugMessageCallback(glDebugOutput, nullptr);
		glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
	}
#endif


#if 0
	{
		int num_exts;
		glGetIntegerv(GL_NUM_EXTENSIONS, &num_exts);

		printf("GL extensions: ");
		for (int i = 0; i < num_exts; i++) {
			const char* ext = (const char*) glGetStringi(GL_EXTENSIONS, i);
			printf("%s ", ext);
		}
		printf("\n");
	}
#endif

	{
		int max_texture_size;
		glGetIntegerv(GL_MAX_TEXTURE_SIZE, &max_texture_size);
		log_info("GL max texture size = %d", max_texture_size);

		log_info("GL vendor: %s",                   (const char*)glGetString(GL_VENDOR));
		log_info("GL renderer: %s",                 (const char*)glGetString(GL_RENDERER));
		log_info("GL version: %s",                  (const char*)glGetString(GL_VERSION));
		log_info("GL shading language version: %s", (const char*)glGetString(GL_SHADING_LANGUAGE_VERSION));
	}

	SDL_GL_SetSwapInterval(0);

	glDisable(GL_CULL_FACE);
	// glEnable(GL_CULL_FACE);
	// glCullFace(GL_FRONT);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glDisable(GL_DEPTH_TEST);

	// glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	// glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);


	// Create game framebuffer
	{
		glGenTextures(1, &game_texture); // @Leak

		glBindTexture(GL_TEXTURE_2D, game_texture);
		defer { glBindTexture(GL_TEXTURE_2D, 0); };

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); // GL_LINEAR for sharp bilinear shader
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, GAME_W * GAME_TEXTURE_SCALE, GAME_H * GAME_TEXTURE_SCALE, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
	}

	{
		glGenTextures(1, &game_depth_texture); // @Leak

		glBindTexture(GL_TEXTURE_2D, game_depth_texture);
		defer { glBindTexture(GL_TEXTURE_2D, 0); };

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F, GAME_W * GAME_TEXTURE_SCALE, GAME_H * GAME_TEXTURE_SCALE, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
	}
	
	{
		glGenFramebuffers(1, &game_fbo); // @Leak

		glBindFramebuffer(GL_FRAMEBUFFER, game_fbo);
		defer { glBindFramebuffer(GL_FRAMEBUFFER, 0); };

		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, game_texture,       0);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,  GL_TEXTURE_2D, game_depth_texture, 0);
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

		arena = arena_create(all_memory);

		// frame_arena = arena_create(1000);
	}

	Mix_Init(MIX_INIT_MP3);

	{
		SDL_version cver;
		MIX_VERSION(&cver);
		log_info("Compiled against SDL_mixer %u.%u.%u", cver.major, cver.minor, cver.patch);

		const SDL_version* lver = Mix_Linked_Version();
		log_info("Linked against SDL_mixer %u.%u.%u", lver->major, lver->minor, lver->patch);
	}

	Mix_OpenAudio(MIX_DEFAULT_FREQUENCY, MIX_DEFAULT_FORMAT, MIX_DEFAULT_CHANNELS, 2048);

	{
		int freq;
		u16 format;
		int nchannels;
		Mix_QuerySpec(&freq, &format, &nchannels);

		log_info("Got Audio Frequency %d", freq);
		log_info("Got Audio Format %u",    format);
		log_info("Got Audio Channels %d",  nchannels);

		printf("INFO: Available chunk decoders: ");
		for (int i = 0, n = Mix_GetNumChunkDecoders(); i < n; i++) {
			printf("%s ", Mix_GetChunkDecoder(i));
		}
		printf("\n");

		printf("INFO: Available music decoders: ");
		for (int i = 0, n = Mix_GetNumMusicDecoders(); i < n; i++) {
			printf("%s ", Mix_GetMusicDecoder(i));
		}
		printf("\n");
	}

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

	// log_info("Read package header in %fms.", (GetTime() - loading_time) * 1000.0);
	loading_time = GetTime();

	// load textures
	{
		static_assert(NUM_TEXTURES == 11, "");

		// @Leak
		texture_data[tex_atlas_0]                     = load_texture("textures/atlas_0.png", FILTER_FOR_SPRITES);
		texture_data[tex_eosd_misty_lake]             = load_texture("textures/eosd_misty_lake.png", GL_LINEAR, GL_MIRRORED_REPEAT);
		texture_data[tex_cirno_spellcard_background]  = load_texture("textures/cirno_spellcard_background.png");
		texture_data[tex_background]                  = load_texture("textures/background.png");
		texture_data[tex_white]                       = load_texture("textures/white.png");
		texture_data[tex_boss_cirno_portrait]         = load_texture("textures/boss_cirno_portrait.png");
		texture_data[tex_boss_youmu_portrait]         = load_texture("textures/boss_youmu_portrait.png");
		texture_data[tex_spellcard_attack_anim_label] = load_texture("textures/spellcard_attack_anim_label.png");
		texture_data[tex_pcb_youmu_stairs]            = load_texture("textures/pcb_youmu_stairs.png");
		texture_data[tex_pcb_youmu_bg]                = load_texture("textures/pcb_youmu_bg.png");
		texture_data[tex_pcb_youmu_bg_flowers]        = load_texture("textures/pcb_youmu_bg_flowers.png");
	}

	log_info("Loaded textures in %fms.", (GetTime() - loading_time) * 1000.0);
	loading_time = GetTime();

	// load sounds
	{

		// @Leak
		for (int i = 0; i < NUM_SOUNDS; i++) {
			sound_data[i] = load_sound(sounds_filenames[i]);
		}

		if (sound_data[snd_enemy_shoot]) Mix_VolumeChunk(sound_data[snd_enemy_shoot], (int)(MIX_MAX_VOLUME * 0.50f));
	}

	log_info("Loaded sounds in %fms.", (GetTime() - loading_time) * 1000.0);
	loading_time = GetTime();

	// load shaders
	{
		r = &renderer;
		r->load_shaders();
	}

	log_info("Loaded shaders in %fms.", (GetTime() - loading_time) * 1000.0);
	loading_time = GetTime();

	package.close();

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

	console.init();

}

void Game::destroy() {

	console.destroy();

	if (music) Mix_FreeMusic(music);
	music = nullptr;

	switch (state) {
		case STATE_TITLE_SCREEN: title_screen.destroy(); break;
		case STATE_PLAYING:      w->destroy();           break;
	}

	r->destroy();

	package.destroy();

	Mix_CloseAudio();
	Mix_Quit();

	arena_destroy(&frame_arena);
	arena_destroy(&arena);

	SDL_GL_DeleteContext(gl_context);

	SDL_DestroyWindow(window);

	SDL_Quit();
}

double GetTime() {
	return (double)SDL_GetPerformanceCounter() / (double)SDL_GetPerformanceFrequency();
}

#define TARGET_FPS 60

void Game::run() {

	double prev_time = GetTime() - (1.0 / (double)TARGET_FPS);

	while (!quit) {

		double time = GetTime();
		double frame_end_time = time + (1.0 / (double)TARGET_FPS);
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

		float delta = 60.0f / (float)TARGET_FPS;

		// @Todo: Coroutines broken
		if (is_key_held(SDL_SCANCODE_F)) delta *= 2.0f;
		if (is_key_held(SDL_SCANCODE_S)) delta *= 0.5f;

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
			SDL_Delay((u32)(time_to_sleep * 1000.0));

			while (GetTime() < frame_end_time) {}
		}
	}

}

void Game::update(float delta) {
	double time = GetTime();

	if (!skip_frame || state == STATE_PLAYING) {
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


	int backbuffer_width;
	int backbuffer_height;
	SDL_GL_GetDrawableSize(window, &backbuffer_width, &backbuffer_height);

	{
		float xscale = backbuffer_width  / (float)GAME_W;
		float yscale = backbuffer_height / (float)GAME_H;
		game_texture_scale = min(xscale, yscale);

		game_texture_pos.w = (int) (GAME_W * game_texture_scale);
		game_texture_pos.h = (int) (GAME_H * game_texture_scale);
		game_texture_pos.x = (backbuffer_width  - game_texture_pos.w) / 2;
		game_texture_pos.y = (backbuffer_height - game_texture_pos.h) / 2;

#if RENDER_GAME_TO_TEXTURE
		game_viewport.x = 0;
		game_viewport.y = 0;
		game_viewport.w = GAME_W * GAME_TEXTURE_SCALE;
		game_viewport.h = GAME_H * GAME_TEXTURE_SCALE;
		game_viewport_scale = GAME_TEXTURE_SCALE;
#else
		game_viewport = game_texture_pos;
		game_viewport_scale = game_texture_scale;
#endif

		play_area_viewport.x = game_viewport.x + PLAY_AREA_X * game_viewport_scale;
		play_area_viewport.y = game_viewport.y + PLAY_AREA_Y * game_viewport_scale;
		play_area_viewport.w = PLAY_AREA_W * game_viewport_scale;
		play_area_viewport.h = PLAY_AREA_H * game_viewport_scale;
		play_area_viewport_scale = game_viewport_scale;
	}

	// 
	// Render the game to a framebuffer.
	// 
	{
#if RENDER_GAME_TO_TEXTURE
		glBindFramebuffer(GL_FRAMEBUFFER, game_fbo);
		defer { glBindFramebuffer(GL_FRAMEBUFFER, 0); };

		// glClearColor(0, 0, 0, 1);
		// glClear(GL_COLOR_BUFFER_BIT);
#endif

		switch (state) {
			case STATE_TITLE_SCREEN: title_screen.draw(delta); break;
			case STATE_PLAYING:
				w->draw(delta); // Projection matrix will change after this call
				break;
		}
	}

	glViewport(0, 0, backbuffer_width, backbuffer_height);
	r->proj = glm::ortho<float>(0, backbuffer_width, backbuffer_height, 0);

#if RENDER_GAME_TO_TEXTURE
	glClearColor(0, 0, 0, 1);
	glClear(GL_COLOR_BUFFER_BIT);
#endif

	// 
	// Render the framebuffer to screen.
	// 
	{

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
				glUniform2f(u_source_size, GAME_W * GAME_TEXTURE_SCALE, GAME_H * GAME_TEXTURE_SCALE);
			}

			{
				float int_scale = max(floorf(game_texture_scale / (float)GAME_TEXTURE_SCALE), 1.0f);

				int u_scale = glGetUniformLocation(program, "u_Scale");
				glUniform2f(u_scale, int_scale, int_scale);
			}

#if RENDER_GAME_TO_TEXTURE
			r->draw_texture(&t, {},
							{game_texture_pos.x, game_texture_pos.y}, {game_texture_scale, game_texture_scale},
							{}, 0,
							color_white, {false, true});
#endif

			r->break_batch();

			r->current_texture_shader = old_texture_shader;
		}

		auto world_to_screen_x = [&](float x) {
			return (x + PLAY_AREA_X) * game_texture_scale + game_texture_pos.x;
		};

		auto world_to_screen_y = [&](float y) {
			return (y + PLAY_AREA_Y) * game_texture_scale + game_texture_pos.y;
		};

		vec2 pos = {};

		if (show_debug_info) {
			char buf[256];
			string str = Sprintf(buf,
								 "fps: %.2f\n"
								 "update: %.2fms\n"
								 "draw: %.2fms\n"
								 "arena: "       Size_Fmt " / " Size_Fmt "\n"
								 "frame arena: " Size_Fmt " / " Size_Fmt "\n"
								 "draw calls: %d\n"
								 "max batch: %zu / %zu\n"
#if TH_DEBUG
								 "COMPILED WITH DEBUG\n"
#endif
								 "__cplusplus=%ld\n",
								 fps,
								 update_took * 1000.0,
								 draw_took   * 1000.0,
								 Size_Arg(arena.count),       Size_Arg(arena.capacity),
								 Size_Arg(frame_arena.count), Size_Arg(frame_arena.capacity),
								 draw_calls,
								 max_batch, BATCH_MAX_VERTICES,
								 __cplusplus);
			pos = r->draw_text(GetSprite(spr_font_main), str, pos.x, pos.y);
			pos.y += 8;

			// Audio Debug
			{
				auto find_chunk_filename = [&](Mix_Chunk* chunk) -> string {
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
					string filename = find_chunk_filename(chunk);

					char buf[64];
					string str = Sprintf(buf, "%d " Str_Fmt "\n", i, Str_Arg(filename));

					vec4 color = Mix_Playing(i) ? color_white : vec4{0.5f, 0.5f, 0.5f, 1};
					pos = r->draw_text(GetSprite(spr_font_main), str, pos.x, pos.y, HALIGN_LEFT, VALIGN_TOP, color);
				}

				pos.y += 8;
			}

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
									 w->bullets.count,   w->bullets.capacity,
									 w->enemies.count,   w->enemies.capacity,
									 w->p_bullets.count, w->p_bullets.capacity,
									 w->pickups.count,   w->pickups.capacity,
									 w->next_instance_id, (u64)UINT32_MAX,
									 Size_Arg(w->coro_memory),
									 w->animations.count, w->animations.capacity,
									 Size_Arg(w->temp_arena_for_boss.count), Size_Arg(w->temp_arena_for_boss.capacity));
				pos = r->draw_text(GetSprite(spr_font_main), str, pos.x, pos.y);
				pos.y += 8;

#define Object_Fmt "id: %" PRIX64 "\n"

#define Object_Arg(o) (o)->id

				{
					Player* p = &w->player;

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
					r->draw_text(GetSprite(spr_font_main), str, world_to_screen_x(p->x), world_to_screen_y(p->y));
				}

				if (!(w->boss.flags & FLAG_INSTANCE_DEAD)) {

					Boss* b = &w->boss;

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
					r->draw_text(GetSprite(spr_font_main), str, world_to_screen_x(b->x), world_to_screen_y(b->y));
				}
			}

			
		}

		if (frame_advance) {
			string str = "F5 - Next Frame\nF6 - Disable Frame Advance Mode\n";
			pos = r->draw_text(GetSprite(spr_font_main), str, pos.x, pos.y);
		}

		console.draw(delta);

		r->break_batch();
	}

	draw_took = GetTime() - time;
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

Texture load_texture(string fname, int filter, int wrap) {

	auto create_texture = [&](void* pixel_data, int width, int height, int num_channels) -> Texture {
		u32 texture;
		glGenTextures(1, &texture);

		glBindTexture(GL_TEXTURE_2D, texture);
		defer { glBindTexture(GL_TEXTURE_2D, 0); };

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, (filter != -1) ? filter : GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, (filter != -1) ? filter : GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, (wrap != -1) ? wrap : GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, (wrap != -1) ? wrap : GL_REPEAT);

		Assert(num_channels == 4);

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixel_data);

		return {texture, width, height};
	};

	Texture result = {};

	size_t filesize;
	u8* filedata = get_file(fname, &filesize);

	if (filedata) {
		if (is_png(filedata, filesize)) {
			int width;
			int height;
			int num_channels;
			void* pixel_data = stbi_load_from_memory(filedata, (int)filesize, &width, &height, &num_channels, 4);
			defer { if (pixel_data) stbi_image_free(pixel_data); };

			result = create_texture(pixel_data, width, height, num_channels);
		} else if (is_qoi(filedata, filesize)) {
			qoi_desc desc;
			void* pixel_data = qoi_decode(filedata, (int)filesize, &desc, 4);
			defer { if (pixel_data) free(pixel_data); };

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

Mix_Chunk* load_sound(string fname) {
	Mix_Chunk* result = nullptr;

	size_t filesize;
	u8* filedata = get_file(fname, &filesize);

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

u32 load_3d_model_from_obj_file(string fname, int* out_num_vertices) {
	const size_t NUM_POSITIONS = 10'000;
	const size_t NUM_UVS       = 10'000;
	const size_t NUM_NORMALS   = 10'000;
	const size_t NUM_VERTICES  = 10'000;

	const size_t MEMORY =
		NUM_POSITIONS  * sizeof(vec3)
		+ NUM_UVS      * sizeof(vec2)
		+ NUM_NORMALS  * sizeof(vec3)
		+ NUM_VERTICES * sizeof(Vertex);

	Arena arena = arena_create(MEMORY + Kilobytes(1));
	defer { arena_destroy(&arena); };

	auto positions = dynamic_array_cap_from_arena<vec3>   (&arena, NUM_POSITIONS);
	auto uvs       = dynamic_array_cap_from_arena<vec2>   (&arena, NUM_UVS);
	auto normals   = dynamic_array_cap_from_arena<vec3>   (&arena, NUM_NORMALS);
	auto vertices  = dynamic_array_cap_from_arena<Vertex> (&arena, NUM_VERTICES);

	g->package.open();
	defer { g->package.close(); };

	size_t filesize;
	u8* filedata = get_file(fname, &filesize);

	if (!filedata) {
		*out_num_vertices = 0;
		return create_vertex_array_obj(nullptr, 0); // Stub
	}

	string text = {(char*)filedata, filesize};

	while (text.count > 0) {
		string line = eat_line(&text);

		if (line.count == 0) {
			continue;
		}

		if (line[0] == '#') {
			continue;
		}

		eat_whitespace(&line);
		string s = eat_non_whitespace(&line);

		if (s == "v") {
			eat_whitespace(&line);
			string x = eat_non_whitespace(&line);

			eat_whitespace(&line);
			string y = eat_non_whitespace(&line);

			eat_whitespace(&line);
			string z = eat_non_whitespace(&line);

			vec3 pos;
			pos.x = string_to_f32(x);
			pos.y = string_to_f32(y);
			pos.z = string_to_f32(z);

			array_add(&positions, pos);
		} else if (s == "vn") {
			eat_whitespace(&line);
			string x = eat_non_whitespace(&line);

			eat_whitespace(&line);
			string y = eat_non_whitespace(&line);

			eat_whitespace(&line);
			string z = eat_non_whitespace(&line);

			vec3 normal;
			normal.x = string_to_f32(x);
			normal.y = string_to_f32(y);
			normal.z = string_to_f32(z);

			array_add(&normals, normal);
		} else if (s == "vt") {
			eat_whitespace(&line);
			string u = eat_non_whitespace(&line);

			eat_whitespace(&line);
			string v = eat_non_whitespace(&line);

			vec2 uv;
			uv.x = string_to_f32(u);
			uv.y = string_to_f32(v);

			array_add(&uvs, uv);
		} else if (s == "f") {
			eat_whitespace(&line);
			string vert1 = eat_non_whitespace(&line);

			eat_whitespace(&line);
			string vert2 = eat_non_whitespace(&line);

			eat_whitespace(&line);
			string vert3 = eat_non_whitespace(&line);

			auto add_vert = [&](string vert) {
				string pos = eat_numeric(&vert);
				advance(&vert); // Skip '/'

				string uv = eat_numeric(&vert);
				advance(&vert); // Skip '/'

				string normal = eat_numeric(&vert);

				Vertex v;
				v.pos    = positions[string_to_u32(pos)    - 1];
				v.normal = normals  [string_to_u32(normal) - 1];
				v.uv     = uvs      [string_to_u32(uv)     - 1];
				v.color  = color_white;

				array_add(&vertices, v);
			};

			add_vert(vert1);
			add_vert(vert2);
			add_vert(vert3);
		}
	}

	*out_num_vertices = (int) vertices.count;
	return create_vertex_array_obj(vertices.data, vertices.count);
};

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

u8*    get_file(string filename, size_t* out_filesize) { return g->package.get_file(filename, out_filesize); }
string get_file_string(string filename)                { return g->package.get_file_string(filename);        }
