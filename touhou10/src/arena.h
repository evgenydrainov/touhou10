#pragma once

#include "common.h"

#define is_power_of_two(x) ((x) != 0 && ((x) & ((x) - 1)) == 0)

static uintptr_t align_forward(uintptr_t ptr, size_t align) {
	Assert(is_power_of_two(align));
	return (ptr + (align - 1)) & ~(align - 1);
}

struct Arena {
	static constexpr size_t DEFAULT_ALIGNMENT = sizeof(void*);

	u8*    data;
	size_t count;
	size_t capacity;

	void destroy() {
		free(data);
		data     = nullptr;
		count    = 0;
		capacity = 0;
	}

	void clear() {
		count = 0;
	}
};

static Arena ArenaAlloc(size_t _capacity) {
	Arena a;
	a.data     = (u8*) malloc(_capacity);
	a.count    = 0;
	a.capacity = _capacity;
	Assert(a.data);
	return a;
}

static void ArenaRelease(Arena* a) {
	free(a->data);
	a->data     = nullptr;
	a->count    = 0;
	a->capacity = 0;
}

static u8* ArenaPush(Arena* a, size_t size, size_t alignment = Arena::DEFAULT_ALIGNMENT) {
	uintptr_t curr_ptr = (uintptr_t)a->data + (uintptr_t)a->count;
	uintptr_t offset = align_forward(curr_ptr, alignment);
	offset -= (uintptr_t)a->data;

	Assert(offset + size <= a->capacity && "Arena ran out of space");

	u8* ptr = a->data + offset;
	a->count = offset + size;

	return ptr;
}

static Arena ArenaAllocFromArena(Arena* arena, size_t _capacity) {
	Arena a;
	a.data     = ArenaPush(arena, _capacity);
	a.count    = 0;
	a.capacity = _capacity;
	return a;
}

template <typename T>
static T* PushStructN(Arena* a, size_t count) {
	return (T*) ArenaPush(a, sizeof(T) * count);
}

template <typename T>
static T* PushStruct(Arena* a) {
	return PushStructN<T>(a, 1);
}
