#pragma once

#include "common.h"

#define MAX_PARTICLES 1'000

// 
// Maybe a particle can hold a type_index to get some params?
// Or have a separate array for particles of each type.
// 
// Or write a shader for each type so these params will become uniforms.
// 
// I don't know how to write a particle system.
// 

struct Particle {
	float x;
	float y;
	float spd;
	float dir;
	float acc;

	float lifetime;
	float lifespan = 60;

	u32 sprite_index;
	float frame_index;

	vec4 color_from = {1, 1, 1, 1};
	vec4 color_to   = {1, 1, 1, 1};

	vec2 scale_from = {1, 1};
	vec2 scale_to   = {1, 1};
};

struct Particle_System {
	dynamic_array_cap<Particle> particles;

	void init();
	void destroy();

	void update(float delta);
	void draw(float delta);
};
