#include "particle_system.h"

#include "game.h"
#include "cpml.h"

void Particle_System::init() {
	particles = ArrayAllocFromArena<Particle>(&g->arena, MAX_PARTICLES);
}

void Particle_System::destroy() {}

void Particle_System::create_particle(glm::vec2 pos, const Particle_Type& type) {
	Particle p = {};

	float speed     = g->random_visual.rangef(type.speed_min,     type.speed_max);
	float direction = g->random_visual.rangef(type.direction_min, type.direction_max);
	float lifespan  = g->random_visual.rangef(type.lifespan_min,  type.lifespan_max);

	p.pos            = pos;
	p.speed          = speed;
	p.speed_incr     = type.speed_incr;
	p.direction      = direction;
	p.direction_incr = type.direction_incr;
	p.lifespan       = lifespan;
	p.sprite_index   = type.sprite_index;
	p.color          = type.color;

	particles.add(p);
}

void Particle_System::update(float delta) {
	For (p, particles) {
		if (p->lifetime >= p->lifespan) {
			Remove(p, particles);
			continue;
		}

		p->pos.x += lengthdir_x(p->speed, p->direction) * delta;
		p->pos.y += lengthdir_y(p->speed, p->direction) * delta;

		p->speed     += p->speed_incr     * delta;
		p->direction += p->direction_incr * delta;

		p->frame_index = object_animate(p->sprite_index, p->frame_index, delta);

		p->lifetime += delta;
	}
}

void Particle_System::draw(float delta) {
	For (p, particles) {
		r->draw_sprite(GetSprite(p->sprite_index), (int)p->frame_index, p->pos, {1, 1}, 0, p->color);
	}
}
