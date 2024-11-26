#include "game.h"

#include "window_creation.h"
#include "renderer.h"
#include "package.h"
#include "console.h"
#include "util.h"

#include <qoi/qoi.h>

#include <inttypes.h> // for PRIX64
#include <string.h> // for memset

Game game;

static_assert(NUM_SOUNDS == 17, "");
static const char* sounds_filenames[] = {
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

		log_info("Available chunk decoders:");
		for (int i = 0, n = Mix_GetNumChunkDecoders(); i < n; i++) {
			log_info("%s", Mix_GetChunkDecoder(i));
		}

		log_info("Available music decoders:");
		for (int i = 0, n = Mix_GetNumMusicDecoders(); i < n; i++) {
			log_info("%s ", Mix_GetMusicDecoder(i));
		}
	}

	{
		float master_volume = 0.50f;
		float sound_volume  = 0.50f;
		float music_volume  = 0.75f;

		char* env_music_volume = SDL_getenv("TH_MUSIC_VOLUME"); // @Leak
		if (env_music_volume) {
			music_volume = (float) SDL_atof(env_music_volume);
		}

		Mix_Volume(-1,  (int)(MIX_MAX_VOLUME * (master_volume * sound_volume)));
		Mix_VolumeMusic((int)(MIX_MAX_VOLUME * (master_volume * music_volume)));
	}

	double loading_time = get_time();

	// load textures
	{
		static_assert(NUM_TEXTURES == 13, "");

		// @Leak
		load_texture_from_file(&texture_data[tex_atlas_0],                     "textures/atlas_0.png", FILTER_FOR_SPRITES);
		load_texture_from_file(&texture_data[tex_eosd_misty_lake],             "textures/eosd_misty_lake.png", GL_LINEAR, GL_MIRRORED_REPEAT);
		load_texture_from_file(&texture_data[tex_cirno_spellcard_background],  "textures/cirno_spellcard_background.png");
		load_texture_from_file(&texture_data[tex_background],                  "textures/background.png", GL_NEAREST, GL_REPEAT);
		load_texture_from_file(&texture_data[tex_white],                       "textures/white.png");
		load_texture_from_file(&texture_data[tex_boss_cirno_portrait],         "textures/boss_cirno_portrait.png");
		load_texture_from_file(&texture_data[tex_boss_youmu_portrait],         "textures/boss_youmu_portrait.png");
		load_texture_from_file(&texture_data[tex_spellcard_attack_anim_label], "textures/spellcard_attack_anim_label.png");
		load_texture_from_file(&texture_data[tex_pcb_youmu_stairs],            "textures/pcb_youmu_stairs.png");
		load_texture_from_file(&texture_data[tex_pcb_youmu_bg],                "textures/pcb_youmu_bg.png");
		load_texture_from_file(&texture_data[tex_pcb_youmu_bg_flowers],        "textures/pcb_youmu_bg_flowers.png");
		load_texture_from_file(&texture_data[tex_font_main],                   "textures/font_main.png");

		load_texture_from_file(&texture_data[tex_font_cirno], "fonts/cirno_0.png");
	}

	log_info("Loaded textures in %fms.", (get_time() - loading_time) * 1000.0);
	loading_time = get_time();

	// load sounds
	{
		// @Leak
		for (int i = 0; i < NUM_SOUNDS; i++) {
			sound_data[i] = load_sound(sounds_filenames[i]);
		}

		if (sound_data[snd_enemy_shoot]) Mix_VolumeChunk(sound_data[snd_enemy_shoot], (int)(MIX_MAX_VOLUME * 0.50f));
	}

	log_info("Loaded sounds in %fms.", (get_time() - loading_time) * 1000.0);
	loading_time = get_time();

	state = STATE_TITLE_SCREEN;
	title_screen.init();

	music = Mix_LoadMUS("music/dbu_the_foolish_girl.mp3");
	Mix_PlayMusic(music, -1);

	{
		load_bmfont_file(&font_data[fnt_cirno], "fonts/cirno.fnt", "fonts/cirno_0.png");
		load_font_from_texture(&font_data[fnt_main], get_texture(tex_font_main), 15, 15, 15, 16, 16);
	}
}

void Game::deinit() {
	if (music) Mix_FreeMusic(music);
	music = nullptr;

	switch (state) {
		case STATE_TITLE_SCREEN: title_screen.deinit(); break;
		case STATE_PLAYING:      world.deinit();        break;
	}

	Mix_CloseAudio();
	Mix_Quit();
}

void Game::update(float delta) {
	double time = get_time();

	skip_frame = frame_advance;

	if (is_key_pressed(SDL_SCANCODE_F1)) {
		show_debug_info ^= true;
	}

	if (is_key_pressed(SDL_SCANCODE_F4)) {
		set_fullscreen(!is_fullscreen());
	}

	if (is_key_pressed(SDL_SCANCODE_F5, true)) {
		frame_advance = true;
		skip_frame = false;
	}

	if (is_key_pressed(SDL_SCANCODE_F6)) {
		frame_advance = false;
	}

	if (is_key_held(SDL_SCANCODE_F)) delta *= 2.0f;
	if (is_key_held(SDL_SCANCODE_S)) delta *= 0.5f;

	if (!skip_frame || state == STATE_PLAYING) {
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
			world.draw(delta); // Projection matrix will change after this call
			break;
		}
	}

	glViewport(0, 0, GAME_W, GAME_H);
	renderer.proj_mat = glm::ortho<float>(0, GAME_W, GAME_H, 0);

	// draw fps
	{
		char buf[10];
		string str = Sprintf(buf, "%.0ffps", roundf(window.avg_fps));
		draw_text(get_font(fnt_main), str, {35 * 16, 29 * 16});
	}

	break_batch();
}


void Game::late_draw(float delta) {
	auto world_to_screen_x = [&](float x) {
		return (x + PLAY_AREA_X) * renderer.game_texture_scale + renderer.game_texture_rect.x;
	};

	auto world_to_screen_y = [&](float y) {
		return (y + PLAY_AREA_Y) * renderer.game_texture_scale + renderer.game_texture_rect.y;
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
							"debug build=%d\n",
							window.fps,
							(window.frame_took - renderer.draw_took) * 1000.0,
							renderer.draw_took * 1000.0,
							renderer.draw_calls,
							renderer.max_batch, BATCH_MAX_VERTICES,
							debug_build);
		pos = draw_text(get_font(fnt_main), str, pos);
		pos.y += 8;

		// Audio Debug
		{
			auto find_chunk_filename = [&](Mix_Chunk* chunk) -> const char* {
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
				const char* filename = find_chunk_filename(chunk);

				char buf[64];
				string str = Sprintf(buf, "%d %s\n", i, filename);

				vec4 color = Mix_Playing(i) ? color_white : vec4{0.5f, 0.5f, 0.5f, 1};
				pos = draw_text(get_font(fnt_main), str, pos, HALIGN_LEFT, VALIGN_TOP, color);
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
								world.bullets.count,   world.bullets.capacity,
								world.enemies.count,   world.enemies.capacity,
								world.p_bullets.count, world.p_bullets.capacity,
								world.pickups.count,   world.pickups.capacity,
								world.next_instance_id, (u64)UINT32_MAX,
								Size_Arg(world.coro_memory),
								world.animations.count, world.animations.capacity,
								Size_Arg(world.temp_arena_for_boss.count), Size_Arg(world.temp_arena_for_boss.capacity));
			pos = draw_text(get_font(fnt_main), str, pos);
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
				draw_text(get_font(fnt_main), str, {world_to_screen_x(p->x), world_to_screen_y(p->y)});
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
				draw_text(get_font(fnt_main), str, {world_to_screen_x(b->x), world_to_screen_y(b->y)});
			}
		}
	}

	if (frame_advance) {
		string str = "F5 - Next Frame\nF6 - Disable Frame Advance Mode\n";
		pos = draw_text(get_font(fnt_main), str, pos);
	}

	break_batch();
}


Mix_Chunk* load_sound(const char* fname) {
	Mix_Chunk* result = nullptr;

	size_t filesize;
	u8* filedata = get_file(fname, &filesize);

	if (filedata) {
		// Goddammit SDL.
		Assert(filesize <= INT_MAX);
		SDL_RWops* src = SDL_RWFromConstMem(filedata, (int) filesize);
		result = Mix_LoadWAV_RW(src, 1);

		log_info("Loaded sound %s", fname);
	} else {
		log_info("Couldn't load sound %s", fname);
	}

	return result;
}

u32 load_3d_model_from_obj_file(const char* fname, int* out_num_vertices) {
	const size_t NUM_POSITIONS = 10'000;
	const size_t NUM_UVS       = 10'000;
	const size_t NUM_NORMALS   = 10'000;
	const size_t NUM_VERTICES  = 10'000;

	auto positions = malloc_bump_array<vec3>(NUM_POSITIONS);
	defer { free(positions.data); };

	auto uvs = malloc_bump_array<vec2>(NUM_UVS);
	defer { free(uvs.data); };

	auto normals = malloc_bump_array<vec3>(NUM_NORMALS);
	defer { free(normals.data); };

	auto vertices = malloc_bump_array<Vertex>(NUM_VERTICES);
	defer { free(vertices.data); };

	size_t filesize;
	u8* filedata = get_file(fname, &filesize);

	if (!filedata) {
		*out_num_vertices = 0;
		//return create_vertex_array_obj(nullptr, 0); // Stub
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

#if defined(DEVELOPER)
bool console_callback(string str, void* userdata) {
	eat_whitespace(&str);
	string command = eat_non_whitespace(&str);

	if (command == "h" || command == "help") {
		string s = "Commands:\n"
			"skip - Skips boss's phase\n"
			"full_power - Get full power\n"
			"life - Get a life\n"
			"kill_player - Kills the player\n";
		console.write(s);
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
			world.player.state = PLAYER_STATE_DYING;
			world.player.timer = PLAYER_DEATH_TIME;
			play_sound(snd_pichuun);
			return true;
		}
	}

	return false;
}
#endif
