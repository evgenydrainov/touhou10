#pragma once

#include "common.h"
#include "arena.h"

// 
// "Array view".
// 
template <typename T>
struct Array {
	T* data;
	size_t count;

	T& operator[](size_t i) {
		Assert(i < count);
		return data[i];
	}

	T* begin() { return data; }
	T* end()   { return data + count; }
};


// 
// This returns an "array view". If you want to
// allocate an empty array from an arena and then append to it,
// then use "ArrayAllocFromArena"
// 
template <typename T>
static Array<T> PushArray(Arena* a, size_t count) {
	T* data = (T*) ArenaPush(a, sizeof(T) * count);
	return {data, count};
}


// 
// The difference from "Array" is that this one also holds a capacity.
// I don't know how to name theese.
// 
template <typename T>
struct Arena_Backed_Array {
	T* data;
	size_t count;
	size_t capacity;

	T& operator[](size_t i) {
		Assert(i < count);
		return data[i];
	}

	T* begin() { return data; }
	T* end()   { return data + count; }

	T* add(const T& val) {
		Assert(count < capacity);
		data[count] = val;
		T* result = &data[count];
		count++;
		return result;
	}

	T* remove(T* it) {
		Assert(it >= begin());
		Assert(it < end());
		for (T* i = it; i < end() - 1; i++) {
			*i = *(i + 1);
		}
		count--;
		return it;
	}

	void clear() {
		count = 0;
	}
};

template <typename T>
static Arena_Backed_Array<T> ArrayAllocFromArena(Arena* a, size_t capacity) {
	T* data = PushStructN(a, T, capacity);
	return {data, 0, capacity};
}
