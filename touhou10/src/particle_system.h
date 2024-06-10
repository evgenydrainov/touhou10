#pragma once

#include "common.h"
#include "array.h"
#include <glm/glm.hpp>

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
	glm::vec2 pos;
	float speed;
	float speed_incr;
	float direction;
	float direction_incr;
	float lifetime;
	float lifespan;
	u32 sprite_index;
};

struct Particle_Type {
	float speed_min;
	float speed_max;
	float speed_incr;
	float direction_min;
	float direction_max;
	float direction_incr;
	float lifespan_min;
	float lifespan_max;
	u32 sprite_index;
};

struct Particle_System {
	Arena_Backed_Array<Particle> particles;

	void init();
	void destroy();

	void create_particle(glm::vec2 pos, const Particle_Type& type);

	void update(float delta);
	void draw(float delta);
};
