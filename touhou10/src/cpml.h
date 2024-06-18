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
	return (dx * dx + dy * dy) < (r * r);
}

static bool circle_vs_rotated_rect(float circle_x, float circle_y, float circle_radius,
								   float rect_center_x, float rect_center_y, float rect_w, float rect_h, float rect_dir) {
	float dx = circle_x - rect_center_x;
	float dy = circle_y - rect_center_y;

	float x_rotated = rect_center_x - (dx * sin(glm::radians(rect_dir))) - (dy * cos(glm::radians(rect_dir)));
	float y_rotated = rect_center_y + (dx * cos(glm::radians(rect_dir))) - (dy * sin(glm::radians(rect_dir)));

	float x_closest = clamp(x_rotated, rect_center_x - rect_w / 2.0f, rect_center_x + rect_w / 2.0f);
	float y_closest = clamp(y_rotated, rect_center_y - rect_h / 2.0f, rect_center_y + rect_h / 2.0f);

	dx = x_closest - x_rotated;
	dy = y_closest - y_rotated;

	return (dx * dx + dy * dy) < (circle_radius * circle_radius);
}

static float lengthdir_x(float len, float dir) {
	return cosf(glm::radians(dir)) * len;
}

static float lengthdir_y(float len, float dir) {
	return -sinf(glm::radians(dir)) * len;
}

static float wrapf(float a, float b) {
	return fmodf((fmodf(a, b) + b), b);
}

template <typename T>
static T wrap(T a, T b) {
	return ((a % b) + b) % b;
}
