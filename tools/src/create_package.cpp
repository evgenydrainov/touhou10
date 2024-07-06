#define _CRT_SECURE_NO_WARNINGS

// 
// TODO:
// Add compression
// 

#include "tools.h"

#ifdef CREATE_PACKAGE


#include "strings.h"


#include <SDL.h>
#include <stdio.h>
#include <vector>


#ifdef _WIN32
#include <direct.h>
#else
#include <unistd.h>
#endif



static String files_to_pack[] = {
	"textures/atlas_0.png",
	"textures/background.png",
	"textures/boss_cirno_portrait.png",
	"textures/boss_youmu_portrait.png",
	"textures/cirno_spellcard_background.png",
	"textures/pcb_youmu_stairs.png",
	"textures/spellcard_attack_anim_label.png",
	"textures/stage_0_bg.png",
	"textures/white.png",

	"sounds/boss_die.wav",
	"sounds/char_reimu_shoot.wav",
	"sounds/enemy_die.wav",
	"sounds/enemy_hurt.wav",
	"sounds/enemy_shoot.wav",
	"sounds/extend.wav",
	"sounds/graze.wav",
	"sounds/lazer.wav",
	"sounds/menu_cancel.wav",
	"sounds/menu_navigate.wav",
	"sounds/menu_ok.wav",
	"sounds/pause.wav",
	"sounds/pichuun.wav",
	"sounds/pickup.wav",
	"sounds/powerup.wav",
	"sounds/spellcard.wav",

	"models/pcb_youmu_stairs.obj",
};


struct Loaded_File {
	u64 offset;
	u64 filesize;
	u8* filedata;
};


std::vector<Loaded_File> loaded_files;


// @Copy
static u8* read_entire_file(const char* fname, size_t* out_size) {
	u8* result = nullptr;

	FILE* f = nullptr;
#ifdef _WIN32
	fopen_s(&f, fname, "rb");
#else
	f = fopen(fname, "rb");
#endif
	Defer { if (f) fclose(f); };

	if (f) {
		fseek(f, 0, SEEK_END);
		size_t size = (size_t) ftell(f);

		result = (u8*) malloc(size);
		Assert(result);

		fseek(f, 0, SEEK_SET);
		fread(result, size, 1, f);

		*out_size = size;
	}

	return result;
}



int main(int argc, char* argv[]) {


#ifdef _WIN32
	_chdir("../touhou10");
#else
	chdir("../touhou10");
#endif


	// 
	// Load files
	// 
	{
		u64 size_of_header_plus_table_of_contents = 0;

		size_of_header_plus_table_of_contents += 4; // magic
		size_of_header_plus_table_of_contents += 4; // version
		size_of_header_plus_table_of_contents += 4; // flags
		size_of_header_plus_table_of_contents += 4; // num_files

		for (int i = 0; i < ArrayLength(files_to_pack); i++) {
			size_of_header_plus_table_of_contents += 4; // filename_length
			size_of_header_plus_table_of_contents += 4; // offset
			size_of_header_plus_table_of_contents += 4; // filesize
			size_of_header_plus_table_of_contents += 4; // flags
			size_of_header_plus_table_of_contents += files_to_pack[i].count; // filename
		}

		u64 offset = size_of_header_plus_table_of_contents;

		for (int i = 0; i < ArrayLength(files_to_pack); i++) {
			Loaded_File file = {};

			Assert(files_to_pack[i].data[files_to_pack[i].count] == 0);
			file.filedata = read_entire_file(files_to_pack[i].data, &file.filesize);
			Assert(file.filedata);

			file.offset = offset;
			offset += file.filesize;

			loaded_files.push_back(file);
		}
	}



	// 
	// Write package
	// 
	{
		FILE* f = fopen("package.dat", "wb");
		Assert(f);
		Defer { fclose(f); };

		// 
		// Package Header
		// 
		{
			const u32 magic = 0x4f484f54;
			fwrite(&magic, sizeof magic, 1, f);
		}

		{
			const u32 version = 1;
			fwrite(&version, sizeof version, 1, f);
		}

		{
			const u32 flags = 0;
			fwrite(&flags, sizeof flags, 1, f);
		}

		{
			const u32 num_files = ArrayLength(files_to_pack);
			fwrite(&num_files, sizeof num_files, 1, f);
		}

		printf("Written Header.\n");

		// 
		// Table of Contents
		// 
		for (int i = 0; i < ArrayLength(files_to_pack); i++) {

			const u32 filename_length = files_to_pack[i].count;
			Assert(files_to_pack[i].count < (size_t)UINT32_MAX);
			fwrite(&filename_length, sizeof filename_length, 1, f);

			const u32 offset = loaded_files[i].offset;
			Assert(loaded_files[i].offset < (u64)UINT32_MAX);
			fwrite(&offset, sizeof offset, 1, f);

			const u32 filesize = loaded_files[i].filesize;
			Assert(loaded_files[i].filesize < (u64)UINT32_MAX);
			fwrite(&filesize, sizeof filesize, 1, f);

			const u32 flags = 0;
			fwrite(&flags, sizeof flags, 1, f);

			fwrite(files_to_pack[i].data, filename_length, 1, f);
		}

		printf("Written Table of Contents.\n");

		// 
		// Filedata
		// 
		for (int i = 0; i < ArrayLength(files_to_pack); i++) {
			fwrite(loaded_files[i].filedata, loaded_files[i].filesize, 1, f);

			printf("Written " Str_Fmt "\n", Str_Arg(files_to_pack[i]));
		}
	}

	return 0;
}


#endif
