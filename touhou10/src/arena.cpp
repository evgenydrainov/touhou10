#include "arena.h"

#include <stdlib.h>
#include <string.h>

#define is_power_of_two(x) ((x != 0) && ((x & (x - 1)) == 0))

static uintptr_t align_forward(uintptr_t ptr, size_t align) {
	Assert(is_power_of_two(align));
	return (ptr + (align - 1)) & ~(align - 1);
}

Arena ArenaAlloc(size_t size) {
	Arena a = {};
	a.base = malloc(size);
	a.size = size;
	return a;
}

Arena ArenaAllocFromArena(Arena* a, size_t size) {
	void* base = ArenaPush(a, size);
	return {base, size};
}

void ArenaRelease(Arena *a) {
	free(a->base);
	*a = {};
}

void *ArenaPush(Arena *a, size_t size, size_t alignment) {
	uintptr_t curr_ptr = (uintptr_t)a->base + (uintptr_t)a->offset;
	uintptr_t offset = align_forward(curr_ptr, alignment);
	offset -= (uintptr_t)a->base;

	Assert(offset + size <= a->size);

	// a->committed += size;
	void *ptr = (uint8_t *)a->base + offset;
	a->offset = offset + size;

	return ptr;
}

void ArenaPop(Arena *a, size_t size) {
	Assert(a->offset >= size);
	a->offset -= size;
	// a->committed -= size;
}

size_t ArenaGetPos(Arena *a) {
	return a->offset;
}

void ArenaSetPosBack(Arena *a, size_t pos) {
	a->offset = pos;
}

void ArenaClear(Arena *a) {
	a->offset = 0;
	// a->committed = 0;
}
