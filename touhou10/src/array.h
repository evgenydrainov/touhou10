#pragma once

#include "common.h"
#include "arena.h"

template <typename T>
struct Arena_Backed_Array {
	T* data;
	size_t count;
	size_t capacity;

	T& operator[](size_t i) {
		Assert(i < count);
		return data[i];
	}

	T* begin() {
		return data;
	}

	T* end() {
		return data + count;
	}

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
static Arena_Backed_Array<T> array_from_arena(Arena* a, size_t capacity) {
	void* data = PushArray(a, T, capacity);
	Arena_Backed_Array<T> result = {};
	result.data = (T*) data;
	result.capacity = capacity;
	return result;
}
