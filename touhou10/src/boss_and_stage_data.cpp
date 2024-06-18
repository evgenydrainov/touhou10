#include "assets.h"

#include "game.h"
#include "cpml.h"

#include <glad/gl.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "scripting.h"

#include "boss_data_cirno.h"
#include "boss_data_daiyousei.h"
#include "boss_data_youmu.h"

#include "stage_0_data.h"
#include "stage_1_data.h"

static_assert(NUM_BOSSES == 3, "");

BossData boss_data[NUM_BOSSES] = {
	{
		/* .name                      = */ "Cirno",
		/* .phases                    = */ boss_cirno_phases,
		/* .phase_count               = */ ArrayLength(boss_cirno_phases),
		/* .type                      = */ BOSS_TYPE_BOSS,
		/* .spr_idle                  = */ spr_boss_cirno_idle,
		/* .spr_left                  = */ spr_boss_cirno_left,
		/* .spr_right                 = */ spr_boss_cirno_right,
		/* .draw_spellcard_background = */ Cirno_Draw_Spellcard_Background,
		/* .portrait_texture          = */ tex_boss_cirno_portrait,
	},

	{
		/* .name             = */ "Daiyousei",
		/* .phases           = */ boss_daiyousei_phases,
		/* .phase_count      = */ ArrayLength(boss_daiyousei_phases),
		/* .type             = */ BOSS_TYPE_MIDBOSS,
		/* .spr_idle         = */ spr_boss_daiyousei_idle,
		/* .spr_left         = */ 0,
		/* .spr_right        = */ 0,
		/* .portrait_texture = */ 0,
	},

	{
		/* .name                      = */ "Youmu",
		/* .phases                    = */ midboss_youmu_phases,
		/* .phase_count               = */ ArrayLength(midboss_youmu_phases),
		/* .type                      = */ BOSS_TYPE_MIDBOSS,
		/* .spr_idle                  = */ spr_boss_youmu_idle,
		/* .spr_left                  = */ spr_boss_youmu_left,
		/* .spr_right                 = */ spr_boss_youmu_right,
		/* .draw_spellcard_background = */ Cirno_Draw_Spellcard_Background,
		/* .portrait_texture          = */ tex_boss_youmu_portrait,
	},
};

BossData* GetBossData(u32 boss_index) {
	Assert(boss_index < ArrayLength(boss_data));
	return &boss_data[boss_index];
}

static_assert(STAGE_COUNT == 2, "");

StageData stage_data[STAGE_COUNT] = {
	{
		/* .script          = */ Stage_0_Script,
		/* .draw_background = */ Stage_0_Draw_Background,
	},
	{
		/* .script          = */ Stage_1_Script,
		/* .draw_background = */ Stage_1_Draw_Background,
	},
};

StageData* GetStageData(u32 stage_index) {
	Assert(stage_index < ArrayLength(stage_data));
	return &stage_data[stage_index];
}
