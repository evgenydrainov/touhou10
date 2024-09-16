#include "objects.h"

#include "game.h"

void object_init(Object* o, ObjType type) {
	o->id = w->get_instance_id(type);
}

void object_cleanup(Player* p) {}

void object_cleanup(Boss* b) {
	coroutine_destroy(&b->co);
}

void object_cleanup(Enemy* e) {
	coroutine_destroy(&e->co);
}

void object_cleanup(Bullet* b) {
	coroutine_destroy(&b->co);
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

void coroutine_create(Coroutine* co, void (*func)(mco_coro*)) {
	mco_result res;

	mco_desc desc = mco_desc_init(func, 0);
	res = mco_create(&co->handle, &desc);
	Assert(res == MCO_SUCCESS);

	float timer = 0;

	res = mco_push(co->handle, &timer, sizeof timer);
	Assert(res == MCO_SUCCESS);
}

void coroutine_destroy(Coroutine* co) {
	if (co->handle) mco_destroy(co->handle);
	*co = {};
}
