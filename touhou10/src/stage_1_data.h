#pragma once

static void Stage_1_Script(mco_coro* co) {
	Wait(co, 60);

	CreateBoss(MIDBOSS_YOUMU);
}

static void Stage_1_Draw_Background(float delta) {
	r->draw_rectangle({0, 0, PLAY_AREA_W, PLAY_AREA_H}, color_black);
}
