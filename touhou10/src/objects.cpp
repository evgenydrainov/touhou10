#include "objects.h"

#include "game.h"

void object_init(Object* o, ObjType type) {
	o->id = world.get_instance_id(type);
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
	const Sprite& s = get_sprite(sprite_index);

	return sprite_animate(s, frame_index, delta);
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
