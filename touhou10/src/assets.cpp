#include "assets.h"

Texture texture_data[NUM_TEXTURES];

const Texture& get_texture(u32 texture_index) {
	Assert(texture_index < NUM_TEXTURES);
	return texture_data[texture_index];
}

const Sprite& get_sprite(u32 sprite_index) {
	Assert(sprite_index < NUM_SPRITES);
	return sprite_data[sprite_index];
}

Character* GetCharacter(u32 character_index) {
	Assert(character_index < NUM_CHARACTERS);
	return &character_data[character_index];
}

Mix_Chunk* sound_data[NUM_SOUNDS];

Mix_Chunk* GetSound(u32 sound_index) {
	Assert(sound_index < NUM_SOUNDS);
	return sound_data[sound_index];
}

const Font& get_font(u32 font_index) {
	Assert(font_index < NUM_FONTS);
	return font_data[font_index];
}
