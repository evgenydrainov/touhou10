#pragma once

// 
// Cirno's Perfect Math Library.
// 
#include <glm/glm.hpp>

template <typename T>
static T min(T a, T b) {
	return (a < b) ? a : b;
}

template <typename T>
static T max(T a, T b) {
	return (a > b) ? a : b;
}

template <typename T>
static T clamp(T a, T mn, T mx) {
	return max(min(a, mx), mn);
}

template <typename T>
static T lerp(T a, T b, float f) {
	return a + (b - a) * f;
}

static void normalize0(float* vec_px, float* vec_py) {
	float vec_x = *vec_px;
	float vec_y = *vec_py;
	float length = sqrtf(vec_x * vec_x + vec_y * vec_y);
	if (length != 0) {
		*vec_px /= length;
		*vec_py /= length;
	} else {
		*vec_px = 0;
		*vec_py = 0;
	}
}

template <typename T>
static T approach(T start, T end, T shift) {
	return start + clamp(end - start, -shift, shift);
}

static float point_distance(float x1, float y1, float x2, float y2) {
	float dx = x2 - x1;
	float dy = y2 - y1;
	return sqrtf(dx * dx + dy * dy);
}

static float point_direction(float x1, float y1, float x2, float y2) {
	return glm::degrees(atan2f(y1 - y2, x2 - x1));
}

static bool circle_vs_circle(float x1, float y1, float r1, float x2, float y2, float r2) {
	float dx = x2 - x1;
	float dy = y2 - y1;
	float r = r1 + r2;
	return (dx * dx + dy * dy) < r * r;
}

static float lengthdir_x(float len, float dir) {
	return cosf(glm::radians(dir)) * len;
}

static float lengthdir_y(float len, float dir) {
	return -sinf(glm::radians(dir)) * len;
}

static float wrap(float a, float b) {
	return fmodf((fmodf(a, b) + b), b);
}
