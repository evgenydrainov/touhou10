#pragma once

static void Stage_1_Script(mco_coro* co) {
	Wait(60);

	CreateBoss(BOSS_YOUMU_MIDBOSS);
}

static void Stage_1_Draw_Background(float delta) {
	glClearColor(1, 1, 1, 1);
	glClear(GL_COLOR_BUFFER_BIT);
}
