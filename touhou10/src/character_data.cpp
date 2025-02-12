#include "assets.h"

#include "window_creation.h"
#include "game.h"

#include "character_data_reimu.h"

static_assert(NUM_CHARACTERS == 1);

static Character character_data[NUM_CHARACTERS] = {
	{
		/* .name           = */ "Reimu Hakurei",
		/* .move_spd       = */ 3.75f,
		/* .focus_spd      = */ 1.75f,
		/* .radius         = */ 2.0f,
		/* .graze_radius   = */ 18.0f,
		/* .deathbomb_time = */ 15.0f,
		/* .starting_bombs = */ 3,
		/* .shot_type      = */ reimu_shot_type,
		/* .bomb           = */ reimu_bomb,
		/* .spr_idle       = */ spr_reimu_idle,
		/* .spr_left       = */ spr_reimu_left,
		/* .spr_right      = */ spr_reimu_right,
	},
};

Character* GetCharacter(u32 character_index) {
	return &character_data[character_index];
}
