#pragma once

// 
// https://bytesbeneath.com/p/the-arena-custom-memory-allocators
// https://www.rfleury.com/p/untangling-lifetimes-the-arena-allocator
// 

#include "common.h"

struct Arena {
	void *base;
	size_t size;
	size_t offset;
	// size_t committed;
};

Arena ArenaAlloc(size_t size);
void ArenaRelease(Arena *a);

void *ArenaPush(Arena *a, size_t size);
void *ArenaPushZero(Arena *a, size_t size);

#define PushArray(arena, type, count) (type *)ArenaPush((arena), sizeof(type)*(count))
#define PushArrayZero(arena, type, count) (type *)ArenaPushZero((arena), sizeof(type)*(count))
#define PushStruct(arena, type) PushArray((arena), (type), 1)
#define PushStructZero(arena, type) PushArrayZero((arena), (type), 1)

void ArenaPop(Arena *a, size_t size);

size_t ArenaGetPos(Arena *a);

void ArenaSetPosBack(Arena *a, size_t pos);
void ArenaClear(Arena *a);
