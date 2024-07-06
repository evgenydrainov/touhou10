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

static bool is_numeric(char ch) {
	return ch >= '0' && ch <= '9';
}

static bool is_identifier(char ch) {
	return (ch >= 'a' && ch <= 'z')
		|| (ch >= 'A' && ch <= 'Z')
		|| is_numeric(ch)
		|| ch == '_';
}

static void eat_whitespace(String* str) {
	while (str->count > 0 && is_whitespace(*str->data)) {
		str->data++;
		str->count--;
	}
}

static String eat_non_whitespace(String* str) {
	String result = {str->data, 0};

	while (str->count > 0 && !is_whitespace(*str->data)) {
		str->data++;
		str->count--;
		result.count++;
	}

	return result;
}

static String eat_numeric(String* str) {
	String result = {str->data, 0};

	while (str->count > 0 && is_numeric(*str->data)) {
		str->data++;
		str->count--;
		result.count++;
	}

	return result;
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

static String eat_line(String* str) {
	String result = {str->data, 0};

	while (str->count > 0 && *str->data != '\n') {
		str->data++;
		str->count--;
		result.count++;
	}

	// Skip newline character.
	if (str->count > 0) {
		str->data++;
		str->count--;
	}

	return result;
}

static u32 string_to_u32(String str, bool* done = nullptr) {
	if (str.count == 0) {
		if (done) *done = false;
		return 0;
	}

	u32 result = 0;

	for (size_t i = 0; i < str.count; i++) {
		if (!is_numeric(str[i])) {
			if (done) *done = false;
			return 0;
		}

		result *= 10;
		result += str[i] - '0';
	}

	if (done) *done = true;
	return result;
}

static float string_to_f32(String str, bool* done = nullptr) {
	if (str.count == 0) {
		if (done) *done = false;
		return 0;
	}

	float negative = 1;

	while (str.count > 0 && *str.data == '-') {
		negative = -negative;
		str.data++;
		str.count--;
	}

	float result = 0;

	while (str.count > 0 && *str.data != '.') {
		if (!is_numeric(*str.data)) {
			if (done) *done = false;
			return 0;
		}

		result *= 10;
		result += *str.data - '0';

		str.data++;
		str.count--;
	}

	if (str.count > 0 && *str.data == '.') {
		str.data++;
		str.count--;

		float f = 0.1f;

		while (str.count > 0) {
			if (!is_numeric(*str.data)) {
				if (done) *done = false;
				return 0;
			}

			result += (*str.data - '0') * f;
			f /= 10;

			str.data++;
			str.count--;
		}
	}

	if (done) *done = true;
	return result * negative;
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

static void advance(String* str, size_t i = 1) {
	if (i > str->count) {
		i = str->count;
	}

	str->count -= i;
	str->data += i;
}
