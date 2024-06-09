#pragma once

#include "common.h"
#include <glm/glm.hpp>

template <typename T>
struct Keys {
	T* values;
	int num_values;
	float* times;
	int num_times;
	bool discrete;
};

struct SpriteTrack {
	u32 texture_index; // @Temp Use sprites instead
	Keys<glm::vec2> position;
	Keys<glm::vec4> modulate;
	Keys<glm::vec2> scale;
};

struct AnimData {
	SpriteTrack* sprite_tracks;
	int num_sprite_tracks;
	float length;
};

struct Animation {
	AnimData* data;
	float time;
	float speed = 1;

	void draw(float delta);
};
