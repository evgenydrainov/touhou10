#include "objects.h"

#include "game.h"
#include "cpml.h"

#define BOSS_PHASE_START_TIME 60.0f
#define BOSS_SPELL_END_TIME   60.0f

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

				BossPhase* phase = b->GetPhase();
				Assert(phase->script);
				Assert(!b->co);

				mco_desc desc = mco_desc_init(phase->script, 0);
				mco_create(&b->co, &desc);
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

	object_animate(b, delta);

	BossData* data = b->GetData();

	if (b->spd > 0.01f) {
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
	b->wait_timer = BOSS_PHASE_START_TIME;
	b->state = BOSS_WAITING_FOR_PHASE_TO_START;

	if (b->phase_index > 0) {
		LaunchTowardsPoint(b, BOSS_STARTING_X, BOSS_STARTING_Y, 0.05f);
	}

	if (phase->type == PHASE_SPELLCARD) {
		//PlaySound("se_spellcard.wav");
	}
}

void boss_end_phase(Boss* b) {
	For (b, w->bullets) {
		// DropPickup(bullet.x, bullet.y, PICKUP_SCORE);
		object_cleanup(b);
	}
	w->bullets.clear();

	// for (Pickup& pickup : pickups) {
	// 	pickup.homing_target = MAKE_INSTANCE_ID(0, TYPE_PLAYER);
	// }

	Assert(b->co);
	mco_destroy(b->co);
	b->co = nullptr;

	// if (phase_data->type == PHASE_SPELLCARD) {
	// 	// drop some pickups
	// 	for (int i = 0; i < 5; i++) {
	// 		float x = boss.x + random.range(-50.0f, 50.0f);
	// 		float y = boss.y + random.range(-50.0f, 50.0f);
	// 		PickupType type = (i == 4) ? PICKUP_BIGP : PICKUP_POWER;
	// 		DropPickup(x, y, type);
	// 	}
	// }

	BossData* data = b->GetData();
	BossPhase* phase = b->GetPhase();

	if (b->phase_index + 1 < data->phase_count) {
		b->phase_index++;

		if (phase->type == PHASE_SPELLCARD) {
			b->state = BOSS_WAITING_FOR_SPELL_TO_END;
			b->wait_timer = BOSS_SPELL_END_TIME;
		} else {
			boss_start_phase(b);
		}
		//PlaySound("se_enemy_die.wav");
	} else {
		// for (Pickup& pickup : pickups) {
		// 	pickup.homing_target = MAKE_INSTANCE_ID(0, TYPE_PLAYER);
		// }

		if (!data->midboss) {
			//PlaySound("se_boss_die.wav");
			//ScreenShake(6.0f, 120.0f);
		} else {
			//PlaySound("se_enemy_die.wav");
		}

		object_cleanup(b);
		b->flags |= FLAG_INSTANCE_DEAD;
	}
}
