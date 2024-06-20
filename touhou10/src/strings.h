#pragma once

#include "common.h"

#include <stb/stb_sprintf.h> // For Sprintf
#include <stdarg.h>          // For Sprintf

#define Str_Fmt      "%.*s"
#define Str_Arg(str) (unsigned int)(str).count, (str).data

// 
// Not null terminated.
// 
template <size_t N>
struct Static_String {
	static_assert(N > 0, "Capacity must be > 0.");

	static constexpr size_t capacity = N;

	char   data[N];
	size_t count = 0; // Usually you want to leave the buffer uninitialized.

	char& operator[](size_t i)       { Assert(i < count); return data[i]; }
	char  operator[](size_t i) const { Assert(i < count); return data[i]; }
};

struct String {
	char*  data;
	size_t count;

	String() = default;

	String(char* data, size_t count) : data(data), count(count) {}

	template <size_t N>
	String(const char (&arr)[N]) {
		data  = (char*) &arr[0];
		count = N - 1;
	}

	template <size_t N>
	String(Static_String<N>& str) : data(str.data), count(str.count) {}

	char& operator[](size_t i)       { Assert(i < count); return data[i]; }
	char  operator[](size_t i) const { Assert(i < count); return data[i]; }

	bool operator==(const String& other) {
		if (count != other.count) {
			return false;
		}

		for (size_t i = 0; i < count; i++) {
			if (data[i] != other[i]) {
				return false;
			}
		}

		return true;
	}
};

static bool is_whitespace(char ch) {
	return ch == ' ' || ch == '\t' || ch == '\n' || ch == '\r';
}

static bool is_identifier(char ch) {
	return (ch >= 'a' && ch <= 'z')
		|| (ch >= 'A' && ch <= 'Z')
		|| (ch >= '0' && ch <= '9')
		|| ch == '_';
}

static void eat_whitespace(String* str) {
	while (str->count > 0 && is_whitespace(*str->data)) {
		str->data++;
		str->count--;
	}
}

static String eat_identifier(String* str) {
	String result = {str->data, 0};

	while (str->count > 0 && is_identifier(*str->data)) {
		str->data++;
		str->count--;
		result.count++;
	}

	return result;
}

static String eat_next_token(String* str) {
	eat_whitespace(str);
	return eat_identifier(str);
}

static u32 string_to_u32(String str, bool* done = nullptr) {
	if (str.count == 0) {
		if (done) *done = false;
		return 0;
	}

	u32 result = 0;

	for (size_t i = 0; i < str.count; i++) {
		if (str[i] >= '0' && str[i] <= '9') {
			result *= 10;
			result += str[i] - '0';
		} else {
			if (done) *done = false;
			return 0;
		}
	}

	if (done) *done = true;
	return result;
}

template <size_t N>
static void Sprintf(Static_String<N>* str, const char* fmt, ...) {
	va_list va;
	va_start(va, fmt);

	int result = stb_vsnprintf(str->data, str->capacity, fmt, va);
	va_end(va);

	Assert(result > 0);
	str->count = (size_t) result;
}
