#include "package.h"

void Package::init(Arena* a) {
	entries             = dynamic_array_cap_from_arena<Entry>(a, MAX_ENTRIES);
	arena_for_filenames = arena_create_from_arena(a, MEMORY_FOR_FILENAMES);
}

void Package::load(const char* _filename) {
	filename = _filename;

	open();

	// 
	// If "f" is null, then we're loading from disk.
	// So, we don't have to read the header. (And will crash if we try to).
	// 
	if (!f) {
		return;
	}

	SDL_RWseek(f, 0, RW_SEEK_SET);

	{
		u32 magic;
		SDL_RWread(f, &magic, sizeof(magic), 1);

		if (magic != 0x4f484f54) {
			log_error("Package %s has the wrong magic value.", filename);
			return;
		}
	}

	{
		u32 version;
		SDL_RWread(f, &version, sizeof(version), 1);

		if (version != 1) {
			log_error("Version %u of package %s is not supported.", version, filename);
			return;
		}
	}

	{
		u32 flags;
		SDL_RWread(f, &flags, sizeof(flags), 1);
	}

	u32 num_files;
	SDL_RWread(f, &num_files, sizeof(num_files), 1);

	for (u32 i = 0; i < num_files; i++) {
		u32 filename_length;
		SDL_RWread(f, &filename_length, sizeof(filename_length), 1);

		u32 offset;
		SDL_RWread(f, &offset, sizeof(offset), 1);

		u32 filesize;
		SDL_RWread(f, &filesize, sizeof(filesize), 1);

		u32 flags;
		SDL_RWread(f, &flags, sizeof(flags), 1);

		Entry e = {};
		e.offset   = (size_t) offset;
		e.filesize = (size_t) filesize;
		e.flags    = flags;

		e.name.data  = (char*) arena_push(&arena_for_filenames, filename_length);
		e.name.count = filename_length;

		SDL_RWread(f, e.name.data, 1, filename_length);

		array_add(&entries, e);
	}

	log_info("Read %u files from %s.", num_files, filename);
}

void Package::destroy() {
	close();
}

void Package::open() {
	Assert(!f);
	f = SDL_RWFromFile(filename, "rb");

	if (!f) {
		log_info("Couldn't open package %s. Will try to load the files from disk.", filename);
	}

	Assert(!temp_buffer_for_files);
	temp_buffer_for_files = (u8*) malloc(TEMP_BUFFER_FOR_FILES);
}

void Package::close() {
	if (f) SDL_RWclose(f);
	f = nullptr;

	if (temp_buffer_for_files) free(temp_buffer_for_files);
	temp_buffer_for_files = nullptr;
}


u8* Package::get_file(string name, size_t* out_size) {

	auto get_file_from_package = [&](string name, size_t* out_size) -> u8* {
		auto find_entry = [&](string name) -> Entry* {
			For (e, entries) {
				if (e->name == name) {
					return e;
				}
			}
			return nullptr;
		};

		Entry* e = find_entry(name);
		if (!e) {
			log_error("Couldn't find file " Str_Fmt " in %s", Str_Arg(name), filename);
			return nullptr;
		}

		if (!f) {
			log_error("Package must be open to read files.");
			return nullptr;
		}

		Assert(temp_buffer_for_files);

		SDL_RWseek(f, e->offset, RW_SEEK_SET);

		Assert(e->filesize <= TEMP_BUFFER_FOR_FILES);
		SDL_RWread(f, temp_buffer_for_files, 1, e->filesize);

		*out_size = e->filesize;
		return temp_buffer_for_files;
	};

	auto get_file_from_disk = [&](string name, size_t* out_size) -> u8* {

		// @Sad
		char* c_str = to_c_string(name);
		defer { free(c_str); };

		SDL_RWops* src = SDL_RWFromFile(c_str, "rb");
		defer { if (src) SDL_RWclose(src); };

		if (!src) {
			log_error("Couldn't open file \"" Str_Fmt "\"", Str_Arg(name));
			return nullptr;
		}

		if (!temp_buffer_for_files) {
			log_error("Package must be open to read files.");
			return nullptr;
		}

		SDL_RWseek(src, 0, RW_SEEK_END);
		size_t filesize = (size_t) SDL_RWtell(src);

		SDL_RWseek(src, 0, RW_SEEK_SET);

		Assert(filesize <= TEMP_BUFFER_FOR_FILES);
		SDL_RWread(src, temp_buffer_for_files, 1, filesize);

		*out_size = filesize;
		return temp_buffer_for_files;
	};

	if (f) {
		return get_file_from_package(name, out_size);
	} else {
		return get_file_from_disk(name, out_size);
	}
}

string Package::get_file_string(string name) {
	size_t filesize;
	u8* filedata = get_file(name, &filesize);

	if (!filedata) {
		return {};
	}

	return {(char*)filedata, filesize};
}
