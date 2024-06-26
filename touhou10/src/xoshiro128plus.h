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
	uint32_t s[4];

	static uint32_t rotl(const uint32_t x, int k) {
		return (x << k) | (x >> (32 - k));
	}

	uint32_t next(void) {
		const uint32_t result = s[0] + s[3];

		const uint32_t t = s[1] << 9;

		s[2] ^= s[0];
		s[3] ^= s[1];
		s[1] ^= s[2];
		s[0] ^= s[3];

		s[2] ^= t;

		s[3] = rotl(s[3], 11);

		return result;
	}

	// 
	// [a, b)
	// Lower bound inclusive, upper bound exclusive.
	// 
	float rangef(float a, float b) {
		uint32_t x = next();

		float f = (x >> 8) * 0x1.0p-24f;
		Assert(f >= 0);
		Assert(f < 1);

		float result = a + (b - a) * f;
		Assert(result >= a);
		Assert(result < b || (b - a == 0)); // Range can be zero.

		return result;
	}

	template <typename T, size_t N>
	T index(T (&arr)[N]) {
		static_assert(N < (size_t)UINT32_MAX, "");
		return arr[next() % (uint32_t)N];
	}
};
