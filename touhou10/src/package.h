#pragma once

#include "common.h"
#include "array.h"
#include "strings.h"

struct SDL_RWops;

struct Package {

	struct Entry {
		String name;
		size_t offset;
		size_t filesize;
		u32 flags;
	};

	static constexpr size_t MAX_ENTRIES           = 100;
	static constexpr size_t MEMORY_FOR_FILENAMES  = Kilobytes(100);
	static constexpr size_t TEMP_BUFFER_FOR_FILES = Megabytes(1);

	Arena_Backed_Array<Entry> entries;
	Arena arena_for_filenames;
	const char* filename;
	SDL_RWops* f;
	u8* temp_buffer_for_files;

	void init(Arena* a);
	void load(const char* _filename);
	void destroy();

	void open();
	void close();

	u8* get_file(String name, size_t* out_size);

};
