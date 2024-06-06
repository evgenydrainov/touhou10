#pragma once

static void Stage_0_Script(mco_coro* co) {
	Wait(co, 60);

#if 1
	{
		instance_id dai_chan = CreateBoss(MIDBOSS_DAIYOUSEI)->id;
		while (!(w->boss.flags & FLAG_INSTANCE_DEAD) && w->boss.id == dai_chan) {
			Wait(co, 1);
		}
	}

	Wait(co, 60);

	{
		instance_id baka = CreateBoss(BOSS_CIRNO)->id;
		while (!(w->boss.flags & FLAG_INSTANCE_DEAD) && w->boss.id == baka) {
			Wait(co, 1);
		}
	}
#else
	CreateBoss(MIDBOSS_YOUMU);
#endif
}

static void Stage_0_Draw_Background(float delta) {
	static u32 quad_vao;
	static u32 quad_vbo;
	static u32 quad_ebo;

	if (!quad_vao) {
		Assert(!quad_vbo);
		Assert(!quad_ebo);

		glGenVertexArrays(1, &quad_vao);
		glGenBuffers(1, &quad_vbo);
		glGenBuffers(1, &quad_ebo);

		glBindVertexArray(quad_vao);

		float x1 = -100;
		float z1 = -100;
		float x2 = 100;
		float z2 = 100;

		float u1 = 0;
		float v1 = 0;
		float u2 = 1;
		float v2 = 1;

		Vertex vertices[] = {
			{{x1, 0, z1}, {1, 1, 1, 1}, {u1, v1}},
			{{x2, 0, z1}, {1, 1, 1, 1}, {u2, v1}},
			{{x2, 0, z2}, {1, 1, 1, 1}, {u2, v2}},
			{{x1, 0, z2}, {1, 1, 1, 1}, {u1, v2}},
		};

		glBindBuffer(GL_ARRAY_BUFFER, quad_vbo);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

		u32 indices[] = {
			0, 1, 2,
			2, 3, 0,
		};

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, quad_ebo);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, pos));
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, color));
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, uv));
		glEnableVertexAttribArray(2);

		glBindVertexArray(0);
	}

	{
		u32 program = r->shader_stage_0_bg_program;

		glUseProgram(program);

		glm::mat4 model = {1};
		glm::mat4 view = glm::lookAt(glm::vec3{0, 10, 0}, glm::vec3{0, 0, -10}, glm::vec3{0, 1, 0});
		glm::mat4 proj = glm::perspectiveFov(glm::radians(60.0f), (float)PLAY_AREA_W, (float)PLAY_AREA_H, 0.1f, 10'000.0f);

		glm::mat4 MVP = (proj * view) * model;

		int u_MVP = glGetUniformLocation(program, "u_MVP");
		glUniformMatrix4fv(u_MVP, 1, GL_FALSE, &MVP[0][0]);

		int u_Time = glGetUniformLocation(program, "u_Time");
		glUniform1f(u_Time, (float)SDL_GetTicks() / 1000.0f);

		glBindTexture(GL_TEXTURE_2D, GetTexture(tex_stage_0_bg)->ID);
		glBindVertexArray(quad_vao);

		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
		r->draw_calls++;

		glBindVertexArray(0);
		glBindTexture(GL_TEXTURE_2D, 0);

		glUseProgram(0);
	}
}
