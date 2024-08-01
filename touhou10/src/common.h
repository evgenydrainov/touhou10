#pragma once

// -----------------------------------------------------------
// SECTION: Stuff specific to this project.
// -----------------------------------------------------------

#if !defined(TH_DEBUG)
#error "You have to define TH_DEBUG as 1 or 0 whether this is a debug build. (-DTH_DEBUG=1)"
#endif

// -----------------------------------------------------------
// SECTION: Common types.
// -----------------------------------------------------------

#include <SDL.h>
#include <stb/stb_sprintf.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef ptrdiff_t ssize_t;

using glm::vec2;
using glm::vec3;
using glm::vec4;

using glm::mat4;

struct Rect {
	int x;
	int y;
	int w;
	int h;
};

struct Rectf {
	float x;
	float y;
	float w;
	float h;
};

#define ArrayLength(a) (sizeof(a) / sizeof(a[0]))

#define Kilobytes(x) ((size_t)((x) * (size_t)1024))
#define Megabytes(x) Kilobytes((x) * (size_t)1024)
#define Gigabytes(x) Megabytes((x) * (size_t)1024)

// 
// Logging and assert
// 

#define log_info(fmt, ...)  SDL_LogInfo (SDL_LOG_CATEGORY_APPLICATION, fmt, ##__VA_ARGS__)
#define log_error(fmt, ...) SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, fmt, ##__VA_ARGS__)

//
// For now, asserts will be enabled in release build. May help with finding bugs.
//
#define Assert SDL_enabled_assert

#define panic_and_abort(fmt, ...) do { \
	char buf[512]; \
	stb_snprintf(buf, sizeof(buf), "%s:" STRINGIFY(__LINE__) ": " fmt, __FILE__, ##__VA_ARGS__); \
	SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error", buf, g->window); \
	SDL_Quit(); \
	exit(1); \
} while (0)

// 
// For loop
// 

#define For(it, arr)    for (auto it = arr.begin(); it != arr.end(); it++)
#define Remove(it, arr) (it = arr.remove(it), it--)
#define Repeat(n)       for (int CONCAT(_i_, __LINE__) = (int)(n); CONCAT(_i_, __LINE__)--;)

#define CONCAT_INTERNAL(x, y) x##y
#define CONCAT(x, y) CONCAT_INTERNAL(x, y)

#define STRINGIFY_INTERNAL(x) #x
#define STRINGIFY(x) STRINGIFY_INTERNAL(x)

// 
// Defer statement
// Stolen from Jonathan Blow's microsoft_craziness.h
// 

template <typename T>
struct ExitScope {
	T lambda;
	ExitScope(T lambda) : lambda(lambda) {}
	~ExitScope() { lambda(); }
	ExitScope(const ExitScope&);
private:
	ExitScope& operator=(const ExitScope&);
};

class ExitScopeHelp {
public:
	template <typename T>
	ExitScope<T> operator+(T t) { return t; }
};

#define Defer const auto& CONCAT(_defer_, __LINE__) = ExitScopeHelp() + [&]()

// 
// Automatically convert an enum to a string.
// 

#define GENERATE_ENUM(x) x,
#define GENERATE_STRING(x) #x,

#define DEFINE_NAMED_ENUM(Type, List) \
enum Type { List(GENERATE_ENUM) }; \
static const char* Get##Type##Name(Type val) { \
	static const char* names[] = { List(GENERATE_STRING) }; \
	Assert(val >= 0); \
	Assert(val < ArrayLength(names)); \
	return names[val]; \
}

// 
// Human-readable printing for filesizes.
// 

#define Size_Fmt "%.7g %s"
#define Size_Arg(size) format_size_float(size), format_size_string(size)

static float format_size_float(size_t bytes) {
	float result = (float)bytes;
	if (bytes >= 1024) {
		result /= 1024.0f;
		bytes /= 1024;
		if (bytes >= 1024) {
			result /= 1024.0f;
			bytes /= 1024;
			if (bytes >= 1024) {
				result /= 1024.0f;
				bytes /= 1024;
			}
		}
	}
	result = floorf(result * 100.0f) / 100.0f;
	return result;
}

static const char* format_size_string(size_t bytes) {
	const char* result = "bytes";
	if (bytes >= 1024) {
		result = "KB";
		bytes /= 1024;
		if (bytes >= 1024) {
			result = "MB";
			bytes /= 1024;
			if (bytes >= 1024) {
				result = "GB";
				bytes /= 1024;
			}
		}
	}
	return result;
}

// --------------------------------------------------------
// SECTION: Math
// --------------------------------------------------------

// 
// Cirno's Perfect Math Library
// 

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

static vec2 normalize0(vec2 v) {
	float length = sqrtf(v.x * v.x + v.y * v.y);
	if (length != 0) {
		return {v.x / length, v.y / length};
	} else {
		return {0, 0};
	}
}

template <typename T>
static T approach(T start, T end, T shift) {
	return start + clamp(end - start, -shift, shift);
}

static float to_degrees(float rad) { return glm::degrees(rad); }
static float to_radians(float deg) { return glm::radians(deg); }

static float dsin(float deg) { return sinf(to_radians(deg)); }
static float dcos(float deg) { return cosf(to_radians(deg)); }

static float point_distance(float x1, float y1, float x2, float y2) {
	float dx = x2 - x1;
	float dy = y2 - y1;
	return sqrtf(dx * dx + dy * dy);
}

static float point_direction(float x1, float y1, float x2, float y2) {
	return to_degrees(atan2f(y1 - y2, x2 - x1));
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

	float x_rotated = rect_center_x - (dx * dsin(rect_dir)) - (dy * dcos(rect_dir));
	float y_rotated = rect_center_y + (dx * dcos(rect_dir)) - (dy * dsin(rect_dir));

	float x_closest = clamp(x_rotated, rect_center_x - rect_w / 2.0f, rect_center_x + rect_w / 2.0f);
	float y_closest = clamp(y_rotated, rect_center_y - rect_h / 2.0f, rect_center_y + rect_h / 2.0f);

	dx = x_closest - x_rotated;
	dy = y_closest - y_rotated;

	return (dx * dx + dy * dy) < (circle_radius * circle_radius);
}

static float lengthdir_x(float len, float dir) { return  dcos(dir) * len; }
static float lengthdir_y(float len, float dir) { return -dsin(dir) * len; }

static float wrapf(float a, float b) {
	return fmodf((fmodf(a, b) + b), b);
}

template <typename T>
static T wrap(T a, T b) {
	return ((a % b) + b) % b;
}
