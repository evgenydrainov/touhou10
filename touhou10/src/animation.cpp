#include "animation.h"

#include "game.h"
#include "cpml.h"

static glm::vec2 interpolate(glm::vec2 a, glm::vec2 b, float f) {
	return lerp(a, b, f);
}

static glm::vec4 interpolate(glm::vec4 a, glm::vec4 b, float f) {
	return lerp(a, b, f);
}

static bool interpolate(bool a, bool b, float f) {
	Assert(false);
	return false;
}

template <typename T>
static T get_track_value(const Keys<T>& k, float time, T _default = {}) {
	for (int i = k.num_times; i--;) {
		if (time >= k.times[i]) {
			if (i == k.num_times - 1) {
				return k.values[i];
			}

			if (k.discrete) {
				return k.values[i];
			}

			float progress = (time - k.times[i]) / (k.times[i + 1] - k.times[i]);
			return interpolate(k.values[i], k.values[i + 1], progress);
		}
	}

	return _default;
}

void Animation::draw(float delta) {
	for (int i = 0; i < data->num_sprite_tracks; i++) {
		SpriteTrack* sprite_track = &data->sprite_tracks[i];

		glm::vec2 position = get_track_value(sprite_track->position, time, {});
		glm::vec4 modulate = get_track_value(sprite_track->modulate, time, color_white);
		glm::vec2 scale    = get_track_value(sprite_track->scale,    time, {1, 1});

		Texture* t = GetTexture(sprite_track->texture_index);

		glm::vec2 origin = {
			t->width  / 2.0f,
			t->height / 2.0f,
		};

		r->draw_texture(t, {}, position, scale, origin, 0, modulate);
	}
}
