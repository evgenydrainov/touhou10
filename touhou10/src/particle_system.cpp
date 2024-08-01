#include "particle_system.h"

#include "game.h"

void Particle_System::init() {
	particles = ArrayAllocFromArena<Particle>(&g->arena, MAX_PARTICLES);
}

void Particle_System::destroy() {}

void Particle_System::update(float delta) {
	For (p, particles) {
		if (p->lifetime >= p->lifespan) {
			Remove(p, particles);
			continue;
		}

		p->x += lengthdir_x(p->spd, p->dir) * delta;
		p->y += lengthdir_y(p->spd, p->dir) * delta;

		p->spd += p->acc * delta;

		p->frame_index = object_animate(p->sprite_index, p->frame_index, delta);

		p->lifetime += delta;
	}
}

void Particle_System::draw(float delta) {
	For (p, particles) {
		float f = p->lifetime / p->lifespan;

		vec2 scale = lerp(p->scale_from, p->scale_to, f);
		vec4 color = lerp(p->color_from, p->color_to, f);

		r->draw_sprite(GetSprite(p->sprite_index), (int)p->frame_index, {p->x, p->y}, scale, 0, color);
	}
}
