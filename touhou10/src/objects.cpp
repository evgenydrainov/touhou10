#include "objects.h"

#include "game.h"

void object_init(Object* o, ObjType type) {
	o->id = w->get_instance_id(type);
}

void object_cleanup(Player* p) {}

void object_cleanup(Boss* b) {
	if (b->co) {
		mco_destroy(b->co);
		b->co = nullptr;
	}
}

void object_cleanup(Enemy* e) {
	if (e->co) {
		mco_destroy(e->co);
		e->co = nullptr;
	}
}

void object_cleanup(Bullet* b) {
	if (b->co) {
		mco_destroy(b->co);
		b->co = nullptr;
	}
}

void object_cleanup(PlayerBullet* b) {}
void object_cleanup(Pickup* p) {}

float object_animate(u32 sprite_index, float frame_index, float delta) {
	Sprite* s = GetSprite(sprite_index);

	frame_index += s->anim_spd * delta;

	if (frame_index >= (float)s->frame_count) {
		float a = frame_index - (float)s->loop_frame;
		float b = (float)(s->frame_count - s->loop_frame);
		Assert(b != 0);
		frame_index = (float)s->loop_frame + fmodf(a, b);
	}

	return frame_index;
}
