#include "sprite.h"

#include "renderer.h"
#include "assets.h"

void draw_sprite(const Sprite& s, int frame_index, vec2 pos,
				 vec2 scale, float angle,
				 vec4 color, glm::bvec2 flip) {
	const Texture& t = get_texture(s.texture_index);

	Assert(frame_index >= 0);
	Assert(frame_index < s.frame_count);

	const SpriteFrame& frame = s.frames[frame_index];

	draw_texture(t, {frame.u, frame.v, frame.w, frame.h},
				 pos, scale, {(float)s.xorigin, (float)s.yorigin}, angle,
				 color, flip);
}
