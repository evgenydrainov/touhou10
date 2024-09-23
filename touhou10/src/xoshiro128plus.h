#pragma once

/*  Written in 2018 by David Blackman and Sebastiano Vigna (vigna@acm.org)

To the extent possible under law, the author has dedicated all copyright
and related and neighboring rights to this software to the public domain
worldwide. This software is distributed without any warranty.

See <http://creativecommons.org/publicdomain/zero/1.0/>. */

#include <stdint.h>
#include "common.h"

/* This is xoshiro128+ 1.0, our best and fastest 32-bit generator for 32-bit
floating-point numbers. We suggest to use its upper bits for
floating-point generation, as it is slightly faster than xoshiro128**.
It passes all tests we are aware of except for
linearity tests, as the lowest four bits have low linear complexity, so
if low linear complexity is not considered an issue (as it is usually
the case) it can be used to generate 32-bit outputs, too.

We suggest to use a sign test to extract a random Boolean value, and
right shifts to extract subsets of bits.

The state must be seeded so that it is not everywhere zero. */

struct xoshiro128plus {
	u32 s[4];
};

inline u32 random_next(xoshiro128plus* rng) {
	auto rotl = [](const u32 x, int k) -> u32 {
		return (x << k) | (x >> (32 - k));
	};

	const u32 result = rng->s[0] + rng->s[3];

	const u32 t = rng->s[1] << 9;

	rng->s[2] ^= rng->s[0];
	rng->s[3] ^= rng->s[1];
	rng->s[1] ^= rng->s[2];
	rng->s[0] ^= rng->s[3];

	rng->s[2] ^= t;

	rng->s[3] = rotl(rng->s[3], 11);

	return result;
}

// 
// [a, b)
// Lower bound inclusive, upper bound exclusive.
// 
inline float random_rangef(xoshiro128plus* rng, float a, float b) {
	u32 x = random_next(rng); // @Note: don't touch the RNG if range is zero?

	float f = (x >> 8) * 0x1.0p-24f;
	// Assert(f >= 0);
	// Assert(f < 1);

	float result = a + (b - a) * f;
	Assert(result >= a);
	Assert(result < b || (b - a == 0)); // Range can be zero.

	return result;
}

// 
// [a, b)
// Lower bound inclusive, upper bound exclusive.
// 
inline int random_range(xoshiro128plus* rng, int a, int b) {
	u32 x = random_next(rng); // @Note: don't touch the RNG if range is zero?

	if (a > b) {
		int temp = a;
		a = b;
		b = temp;
	}

	int range = b - a;
	if (range == 0) return a;

	int result = a + (x % range);
	return result;
}

template <typename T, size_t N>
inline T random_choose(xoshiro128plus* rng, T (&arr)[N]) {
	static_assert(N >= 1, "");
	static_assert(N < (size_t)UINT32_MAX, "");

	T result = arr[random_next(rng) % (u32)N];

	return result;
}

inline bool random_chance(xoshiro128plus* rng, float chance) {
	float f = random_rangef(rng, 0, 1);
	return f < chance;
}

// 
// Weighted Random
// 

template <typename T>
struct Random_Weight {
	T value;
	float weight;
};

template <typename T, size_t N>
inline T random_weighted(xoshiro128plus* rng, Random_Weight<T> (&arr)[N]) {
	static_assert(N >= 1, "");

	float total_weight = 0;
	for (size_t i = 0; i < N; i++) {
		total_weight += arr[i].weight;
	}

	float f = random_rangef(rng, 0, total_weight);

	for (size_t i = 0; i < N; i++) {
		if (f < arr[i].weight) {
			return arr[i].value;
		}

		f -= arr[i].weight;
	}

	Assert(false);
	return arr[N - 1].value;
}
