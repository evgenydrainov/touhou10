#pragma once

#include "common.h"

struct SpriteFrame {
	int u;
	int v;
	int w;
	int h;
};

struct Sprite {
	u32 texture_index;
	SpriteFrame* frames;
	int frame_count;
	int xorigin;
	int yorigin;
	int loop_frame; // The frame from which animation will loop.
	float anim_spd; // Animation speed
	int width;
	int height;
};

void draw_sprite(const Sprite& s, int frame_index, vec2 pos,
				 vec2 scale = {1, 1}, float angle = 0,
				 vec4 color = color_white, glm::bvec2 flip = {});
