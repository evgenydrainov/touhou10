#pragma once

#include <stdint.h>
#include <math.h>

// 
// Some common stuff.
// 

#if !defined(TH_DEBUG)
#error "You have to define TH_DEBUG as 1 or 0 whether this is a debug build. (-DTH_DEBUG=1)"
#endif

typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

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

#define Kilobytes(x) (size_t)((x) * (size_t)1024)
#define Megabytes(x) Kilobytes((x) * (size_t)1024)
#define Gigabytes(x) Megabytes((x) * (size_t)1024)

#define log_info(fmt, ...) SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, fmt, ##__VA_ARGS__)
#define log_error(fmt, ...) SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, fmt, ##__VA_ARGS__)

#if TH_DEBUG
  #ifdef _WIN32
    #define Assert(cond) while (!(cond)) __debugbreak()
  #else
    #define Assert(cond) while (!(cond)) __builtin_trap()
  #endif
#else
  //
  // @Todo?
  // I feel like assertions shouldn't be excluded from release build.
  //
  #define Assert(cond) while (!(cond)) _Assertion_Failed(__FILE__, __LINE__, #cond)
#endif

void _Assertion_Failed(const char* file, int line, const char* condition);

// 
// "Language extensions".
// 

#define For(it, arr) for (auto it = arr.begin(); it != arr.end(); it++)
#define Remove(it, arr) (it = arr.remove(it), it--)
#define Repeat(n) for (int CONCAT(_i_, __LINE__) = (int)(n); CONCAT(_i_, __LINE__)--;)

#define CONCAT_INTERNAL(x, y) x##y
#define CONCAT(x, y) CONCAT_INTERNAL(x, y)

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
// Nice printing.
// 
// size_t size = Megabytes(10);
// printf("Size is " Size_Fmt "\n", Size_Arg(size));
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
