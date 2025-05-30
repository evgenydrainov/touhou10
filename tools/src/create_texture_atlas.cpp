#define _CRT_SECURE_NO_WARNINGS

// 
// A small tool that packs sprites into an atlas.
// 

#include "tools.h"

#ifdef CREATE_TEXTURE_ATLAS


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

	// 
	// @Todo: this file is encoded in CRLF and there's raw strings and
	// normal strings with "\n". So, sprite_data.cpp should have
	// mixed line endings. But it seems fine?
	// 

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


#endif
