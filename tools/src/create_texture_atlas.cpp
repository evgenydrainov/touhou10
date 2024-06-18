#define _CRT_SECURE_NO_WARNINGS

// 
// A small tool that packs sprites into an atlas.
// 

#include "common.h"
#include "tools.h"

#ifdef CREATE_TEXTURE_ATLAS


#include "cpml.h"

#include <stb/stb_image_write.h>
#include <stb/stb_rect_pack.h>
#include <stb/stb_sprintf.h>

#include <SDL.h>
#include <SDL_image.h>

#ifdef _WIN32
#include <direct.h>
#else
#include <unistd.h>
#endif

#include <vector>
#include <unordered_map>
#include <string>



// @Copy
void _Assertion_Failed(const char* file, int line, const char* condition) {
	char buf[256];
	stb_snprintf(buf, sizeof(buf), "%s:%d:\n%s", file, line, condition);
	SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Assertion failed", buf, nullptr);
	exit(1);
}




struct SpriteFrame {
	int u;
	int v;
	int w;
	int h;
};


struct Create_Sprite {
	const char* name;
	const char* texture;
	int u;
	int v;
	int width;
	int height;
	int xorigin;
	int yorigin;
	int frame_count;
	int frames_in_row;
	float anim_spd;
	int loop_frame;
	int xstride;
	int ystride;

	std::vector<SpriteFrame> frames;
};


static Create_Sprite create_sprites[] = {

	// name                    texture             u    v   w   h   xo  yo  fc  row anim  loop xs  ys
	{"spr_white"             , "white.png",         0,   0, 16, 16,  0,  0,  1,  1},
	{"spr_reimu_idle"        , "characters.png",    0,   0, 32, 48, 16, 24,  8,  8, 0.20f,  0, 32, 48},
	{"spr_reimu_left"        , "characters.png",    0,  48, 32, 48, 16, 24,  8,  8, 0.20f,  4, 32, 48},
	{"spr_reimu_right"       , "characters.png",    0,  96, 32, 48, 16, 24,  8,  8, 0.20f,  4, 32, 48},
	{"spr_font_main"         , "fonts.png",        16,  48, 15, 15,  0,  0, 96, 16, 0.00f,  0, 16, 16},
	{"spr_player_hitbox"     , "characters.png",  192, 144, 64, 64, 32, 32,  1,  1},
	{"spr_enemy_label"       , "fonts.png",        16, 256, 48, 16, 24,  0,  1,  1},
	{"spr_reimu_shot_card"   , "characters.png",    0, 144, 16, 16,  8,  8,  1,  1},
	{"spr_reimu_shot_orb"    , "characters.png",   16, 144, 16, 16,  8,  8,  1,  1},

	{"spr_reimu_orb"         , "characters.png",   64, 144, 16, 16,  8,  8,  3,  3},

	{"spr_pickup"            , "projectiles.png",   0, 608, 16, 16,  8,  8, 16, 16, 0.00f,  0, 16, 16},
	{"spr_particle_graze"    , "graze.png",         0,   0,  8,  8,  4,  4,  4,  4, 0.25f},

	{"spr_lazer"             , "projectiles.png",   0,   0, 16, 16,  8,  0, 16, 16, 0.00f,  0, 16, 16},
	{"spr_bullet_arrow"      , "projectiles.png",   0,  16, 16, 16,  8,  8, 16, 16, 0.00f,  0, 16, 16},
	{"spr_bullet_outline"    , "projectiles.png",   0,  32, 16, 16,  8,  8, 16, 16, 0.00f,  0, 16, 16},
	{"spr_bullet_filled"     , "projectiles.png",   0,  48, 16, 16,  8,  8, 16, 16, 0.00f,  0, 16, 16},
	{"spr_bullet_rice"       , "projectiles.png",   0,  64, 16, 16,  8,  8, 16, 16, 0.00f,  0, 16, 16},
	{"spr_bullet_kunai"      , "projectiles.png",   0,  80, 16, 16,  8,  8, 16, 16, 0.00f,  0, 16, 16},
	{"spr_bullet_shard"      , "projectiles.png",   0,  96, 16, 16,  8,  8, 16, 16, 0.00f,  0, 16, 16},
	{"spr_bullet_card"       , "projectiles.png",   0, 112, 16, 16,  8,  8, 16, 16, 0.00f,  0, 16, 16},
	{"spr_bullet_bullet"     , "projectiles.png",   0, 128, 16, 16,  8,  8, 16, 16, 0.00f,  0, 16, 16},

	{"spr_bullet_small"      , "projectiles.png",   0, 240,  8,  8,  4,  4, 16,  8, 0.00f,  0,  8,  8},

	{"spr_boss_cirno_idle"   , "boss_cirno.png",  176, 160, 64, 64, 32, 32,  4,  4, 0.15f,  0, 64, 64},
	{"spr_boss_cirno_right"  , "boss_cirno.png",  176, 224, 64, 64, 32, 32,  4,  4, 0.15f,  3, 64, 64},
	{"spr_boss_cirno_left"   , "boss_cirno.png",  176, 288, 64, 64, 32, 32,  4,  4, 0.15f,  3, 64, 64},

	{"spr_boss_daiyousei_idle","Enemies.png",     288, 496, 48, 48, 24, 24,  1,  1, 0.00f,  0, 48, 48},

	{"spr_boss_youmu_idle",    "boss_youmu.png",  176, 160, 48, 64, 24, 32,  4,  4, 0.10f},
	{"spr_boss_youmu_left",    "boss_youmu.png",  176, 224, 48, 64, 24, 32,  4,  4, 0.15f,  3},
	{"spr_boss_youmu_right",   "boss_youmu.png",  176, 288, 48, 64, 24, 32,  4,  4, 0.15f,  3},

	// name                    texture             u    v   w   h   xo  yo  fc  row anim  loop xs  ys
	{"spr_fairy_0",            "Enemies.png",     288, 288, 32, 32, 16, 16, 8,   8, 0.10f},
	{"spr_fairy_1",            "Enemies.png",     288, 320, 32, 32, 16, 16, 8,   8, 0.10f},
	{"spr_enemy_0",            "Enemies.png",     416, 384, 32, 32, 16, 16, 1},

};





int main(int argc, char* argv[]) {


	for (int i = 0; i < ArrayLength(create_sprites); i++) {
		Create_Sprite* s = &create_sprites[i];
		if (s->xstride == 0) {
			s->xstride = s->width;
			printf("INFO: %s: xstride = 0\n", s->name);
		}
		if (s->ystride == 0) {
			s->ystride = s->height;
			printf("INFO: %s: ystride = 0\n", s->name);
		}
		if (s->frames_in_row == 0) {
			s->frames_in_row = s->frame_count;
			printf("INFO: %s: frames_in_row = 0\n", s->name);
		}

		Assert(s->name);
		Assert(s->texture);
		Assert(s->width > 0);
		Assert(s->height > 0);
		Assert(s->frame_count > 0);
		Assert(s->frames_in_row > 0);
	}

#ifdef _WIN32
	_chdir("../touhou10");
#else
	chdir("../touhou10");
#endif


	int atlas_index = 0;

	int atlas_width = 1024;
	int atlas_height = 1024;

	int num_nodes = 2000;
	stbrp_node* nodes = (stbrp_node*) calloc(num_nodes, sizeof(*nodes));

	stbrp_context context;
	stbrp_init_target(&context, atlas_width, atlas_height, nodes, num_nodes);

	std::vector<stbrp_rect> rects;

	for (int i = 0; i < ArrayLength(create_sprites); i++) {
		Create_Sprite* s = &create_sprites[i];

		for (int j = 0; j < s->frame_count; j++) {
			stbrp_rect rect = {};
			rect.id = i * 10000 + j;
			rect.w = s->width  + 2;
			rect.h = s->height + 2;

			rects.push_back(rect);
		}

	}

	printf("%zu rects\n", rects.size());

	int res = stbrp_pack_rects(&context, rects.data(), rects.size());

	Assert(res);
	printf("result: %d\n", res);



	SDL_Init(SDL_INIT_VIDEO);
	IMG_Init(IMG_INIT_PNG);

	SDL_Surface* atlas = SDL_CreateRGBSurfaceWithFormat(0, atlas_width, atlas_height, 32, SDL_PIXELFORMAT_ABGR8888);

	SDL_SetSurfaceBlendMode(atlas, SDL_BLENDMODE_NONE);

	std::unordered_map<std::string, SDL_Surface*> loaded_surfs;

	for (int i = 0; i < rects.size(); i++) {
		stbrp_rect rect = rects[i];

		if (!rect.was_packed) {
			continue;
		}

		int sprite_index = rect.id / 10000;
		int frame_index = rect.id % 10000;

		Assert(sprite_index >= 0);
		Assert(sprite_index < ArrayLength(create_sprites));

		Create_Sprite* s = &create_sprites[sprite_index];

		SDL_Surface* surf;
		{
			auto found = loaded_surfs.find(s->texture);
			if (found != loaded_surfs.end()) {
				surf = found->second;
			} else {
				char filepath[256];
				stb_snprintf(filepath, sizeof(filepath), "textures_not_packed/%s", s->texture);

				surf = IMG_Load(filepath);

				loaded_surfs.emplace(s->texture, surf);
			}
		}

		Assert(surf);

		SDL_SetSurfaceBlendMode(surf, SDL_BLENDMODE_NONE);

		// log_info("format: %s", SDL_GetPixelFormatName(surf->format->format));

		auto blit = [&](int xoff, int yoff, int width, int height) {
			int frame_u = s->u + (frame_index % s->frames_in_row) * s->xstride;
			int frame_v = s->v + (frame_index / s->frames_in_row) * s->ystride;

			SDL_Rect src;
			src.x = frame_u + xoff;
			src.y = frame_v + yoff;

			src.x = clamp(src.x, frame_u, frame_u + s->width  - width);
			src.y = clamp(src.y, frame_v, frame_v + s->height - height);

			src.w = width;
			src.h = height;

			SDL_Rect dest;
			dest.x = rect.x + 1 + xoff;
			dest.y = rect.y + 1 + yoff;
			dest.w = width;
			dest.h = height;

			SDL_BlitSurface(surf, &src, atlas, &dest);
		};

		blit(-1,       -1, 1,        1); // top left
		blit(0,        -1, s->width, 1); // top
		blit(s->width, -1, 1,        1); // top right

		blit(-1,       0, 1,        s->height); // left
		blit(0,        0, s->width, s->height); // middle
		blit(s->width, 0, 1,        s->height); // right

		blit(-1,       s->height, 1,        1); // bottom left
		blit(0,        s->height, s->width, 1); // bottom
		blit(s->width, s->height, 1,        1); // bottom right

		printf("Packed %s frame %d.\n", s->name, frame_index);

		SpriteFrame frame = {};
		frame.u = rect.x + 1;
		frame.v = rect.y + 1;
		frame.w = s->width;
		frame.h = s->height;
		if (s->frames.size() < frame_index + 1) {
			s->frames.resize(frame_index + 1);
		}
		s->frames[frame_index] = frame;
	}

	{
		char filepath[256];
		stb_snprintf(filepath, sizeof(filepath), "textures/atlas_%d.png", atlas_index);
		IMG_SavePNG(atlas, filepath);
	}


	FILE* f = fopen("src/sprite_data.cpp", "wb");

	fprintf(f, R"(// 
// GENERATED
// 
#include "assets.h"
)");

	for (int i = 0; i < ArrayLength(create_sprites); i++) {
		Create_Sprite* s = &create_sprites[i];

		fprintf(f, "static SpriteFrame %s_frames[] = {\n", s->name);

		for (int i = 0; i < s->frames.size(); i++) {
			fprintf(f,
					"	{/* .u = */ %d, /* .v = */ %d, /* .w = */ %d, /* .h = */ %d},\n",
					s->frames[i].u, s->frames[i].v, s->frames[i].w, s->frames[i].h);
		}

		fprintf(f, "};\n");
	}

	fprintf(f, "Sprite sprite_data[NUM_SPRITES] = {\n");

	for (int i = 0; i < ArrayLength(create_sprites); i++) {
		Create_Sprite* s = &create_sprites[i];

		fprintf(f, R"(	/* [%s] = */ {
		/* .texture_index = */ tex_atlas_%d,
		/* .frames        = */ %s_frames,
		/* .frame_count   = */ ArrayLength(%s_frames),
		/* .xorigin       = */ %d,
		/* .yorigin       = */ %d,
		/* .loop_frame    = */ %d,
		/* .anim_spd      = */ %ff,
		/* .width         = */ %d,
		/* .height        = */ %d,
	},
)", s->name, atlas_index, s->name, s->name,
s->xorigin, s->yorigin, s->loop_frame, s->anim_spd, s->width, s->height);
	}

	fprintf(f, "};\n");

	fclose(f);





	f = fopen("src/sprite_indices.h", "wb");

	fprintf(f, R"(// 
// GENERATED
// 
enum {
)");

	for (int i = 0; i < ArrayLength(create_sprites); i++) {
		Create_Sprite* s = &create_sprites[i];

		fprintf(f, "	%s,\n", s->name);
	}

	fprintf(f, R"(	NUM_SPRITES,
};
)");

	fclose(f);



	IMG_Quit();
	SDL_Quit();



	return 0;
}

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb/stb_image_write.h>
#undef STB_IMAGE_WRITE_IMPLEMENTATION

#define STB_RECT_PACK_IMPLEMENTATION
#include <stb/stb_rect_pack.h>
#undef STB_RECT_PACK_IMPLEMENTATION

#define STB_SPRINTF_IMPLEMENTATION
#include <stb/stb_sprintf.h>
#undef STB_SPRINTF_IMPLEMENTATION


#endif
