#include "objects.h"

#include "world.h"

#define BOSS_PHASE_START_TIME_SPELL    150.0f
#define BOSS_PHASE_START_TIME_NONSPELL 60.0f
#define BOSS_SPELL_END_TIME            60.0f

void boss_update(Boss* b, float delta) {
	switch (b->state) {
		case BOSS_STATE_NORMAL: {
			b->timer -= delta;
			if (b->timer <= 0.0f || b->hp <= 0.0f) {
				boss_end_phase(b);
			}
			break;
		}

		case BOSS_WAITING_FOR_PHASE_TO_START: {
			b->wait_timer -= delta;
			if (b->wait_timer <= 0.0f) {
				b->state = BOSS_STATE_NORMAL;

				coroutine_destroy(&b->co);

				BossPhase* phase = b->GetPhase();
				if (phase->script) {
					coroutine_create(&b->co, phase->script);
				}
			}
			break;
		}

		case BOSS_WAITING_FOR_SPELL_TO_END: {
			b->wait_timer -= delta;
			if (b->wait_timer <= 0.0f) {
				boss_start_phase(b);
			}
			break;
		}
	}

	b->dir = wrapf(b->dir, 360.0f);

	b->frame_index = object_animate(b->sprite_index, b->frame_index, delta);
	b->flags &= ~FLAG_BOSS_WAS_HIT_THIS_FRAME;

	BossData* data = b->GetData();

	if (b->spd > BOSS_MOVE_THRESHOLD_VISUAL) {
		if (90.0f <= b->dir && b->dir < 270.0f) {
			if (b->sprite_index != data->spr_left) {
				b->sprite_index = data->spr_left;
				b->frame_index = 0;
			}
		} else {
			if (b->sprite_index != data->spr_right) {
				b->sprite_index = data->spr_right;
				b->frame_index = 0;
			}
		}
	} else {
		if (b->sprite_index != data->spr_idle) {
			b->sprite_index = data->spr_idle;
			b->frame_index = 0;
		}
	}
}

void boss_start_phase(Boss* b) {
	BossPhase* phase = b->GetPhase();

	b->hp = phase->hp;
	b->timer = phase->time * 60.0f;
	b->state = BOSS_WAITING_FOR_PHASE_TO_START;

	if (b->phase_index > 0) {
		LaunchTowardsPoint(b, BOSS_STARTING_X, BOSS_STARTING_Y, 0.05f);
	}

	if (phase->type == PHASE_SPELLCARD) {
		b->wait_timer = BOSS_PHASE_START_TIME_SPELL;

		play_sound(get_sound(snd_spellcard));

		Animation a = {};
		a.data = &anim_boss_spellcard;
		a.speed = 0.75f;
		anim_boss_spellcard.sprite_tracks[0].texture_index = b->GetData()->portrait_texture; // @Hack
		array_add(&world.animations, a);
	} else {
		b->wait_timer = BOSS_PHASE_START_TIME_NONSPELL;
	}
}

void boss_end_phase(Boss* b) {
	For (b, world.bullets) {
		drop_pickup(b->x, b->y, PICKUP_TYPE_SCORE);

		object_cleanup(b);
	}
	world.bullets.count = 0;

	For (p, world.pickups) {
		p->homing_target = world.player.id;
	}

	world.delta_multiplier = 1;
	world.boss_pcb_youmu_effect = 0;

	coroutine_destroy(&b->co);

	BossData*  data  = b->GetData();
	BossPhase* phase = b->GetPhase();

	if (phase->type == PHASE_SPELLCARD) {
		// Drop some pickups
		for (int i = 0; i < 5; i++) {
			float x = b->x + random_rangef(&world.rng, -50.0f, 50.0f);
			float y = b->y + random_rangef(&world.rng, -50.0f, 50.0f);
			PickupType type = (i == 4) ? PICKUP_TYPE_POWER_BIG : PICKUP_TYPE_POWER;
			drop_pickup(x, y, type);
		}

		boss_create_kira_particle(b);
	}

	if (b->phase_index + 1 < data->phase_count) {
		b->phase_index++;

		if (phase->type == PHASE_SPELLCARD) {
			b->state = BOSS_WAITING_FOR_SPELL_TO_END;
			b->wait_timer = BOSS_SPELL_END_TIME;
		} else {
			boss_start_phase(b);
		}

		play_sound(get_sound(snd_enemy_die));
	} else {
		For (p, world.pickups) {
			p->homing_target = world.player.id;
		}

		if (data->type == BOSS_TYPE_BOSS) {
			play_sound(get_sound(snd_boss_die));
			//ScreenShake(6.0f, 120.0f);
		} else {
			play_sound(get_sound(snd_enemy_die));
		}

		object_cleanup(b);
		b->flags |= FLAG_INSTANCE_DEAD;
	}
}

void boss_create_kira_particle(Object* b) {
	Particle p = {};
	p.pos.x = b->x;
	p.pos.y = b->y;
	p.sprite_index = spr_kira_particle;
	p.lifespan = 30;
	p.scale_from = {2, 2};
	p.scale_to   = {12, 12};
	p.color_from = {1, 1, 1, 1};
	p.color_to   = {1, 1, 1, 0};

	add_particle(p);
}
