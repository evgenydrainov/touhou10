#pragma once

// 
// https://bytesbeneath.com/p/the-arena-custom-memory-allocators
// https://www.rfleury.com/p/untangling-lifetimes-the-arena-allocator
// 

#include "common.h"

#define DEFAULT_ALIGNMENT sizeof(void*)

#define PushStructN(arena, T, count) (T*)ArenaPush((arena), sizeof(T)*(count))
#define PushStruct(arena, T)         PushStructN((arena), (T), 1)

struct Arena {
	void *base;
	size_t size;
	size_t offset;
	// size_t committed;
};

Arena ArenaAlloc(size_t size);
Arena ArenaAllocFromArena(Arena* a, size_t size);
void  ArenaRelease(Arena *a);

void *ArenaPush(Arena *a, size_t size, size_t alignment = DEFAULT_ALIGNMENT);
void ArenaPop(Arena *a, size_t size);

size_t ArenaGetPos(Arena *a);

void ArenaSetPosBack(Arena *a, size_t pos);
void ArenaClear(Arena *a);
