#include "renderer.h"
#include "util.h"
#include "scripting.h"

void Stage_0_Script(mco_coro* co) {
	auto spawn_spinner = [&]() {
		auto Script = [](mco_coro* co) {
			wait(60);
			self->acc = -0.01f;
			while (self->spd > 0) {
				wait(1);
			}
			self->dir = -self->dir;
			self->acc = 0.01f;
		};

		auto OnDeath = [](Object* o) {
			int r = random_range(&world.rng, 0, 3);
			float dir = random_rangef(&world.rng, 0.0f, 360.0f);
			if (r == 0) {
				ShootO(o, 2, dir,       0, spr_bullet_rice, 2);
				ShootO(o, 2, dir +  90, 0, spr_bullet_rice, 2);
				ShootO(o, 2, dir + 180, 0, spr_bullet_rice, 2);
				ShootO(o, 2, dir + 270, 0, spr_bullet_rice, 2);
			} else if (r == 1) {
				ShootO(o, 3, dir,       0, spr_bullet_kunai, 10);
				ShootO(o, 3, dir +  90, 0, spr_bullet_kunai, 10);
				ShootO(o, 3, dir + 180, 0, spr_bullet_kunai, 10);
				ShootO(o, 3, dir + 270, 0, spr_bullet_kunai, 10);
			}
		};

		auto OnUpdate = [](Object* o, float delta) {
			((Enemy*)o)->angle += 10 * delta;
		};

		float x = random_rangef(&world.rng, 0.0f, (float)PLAY_AREA_W);
		float y = 0;

		CreateEnemy(x, y, 2, 270 + random_rangef(&world.rng, -30.0f, 30.0f), 0, spr_enemy_0, 10, 1, Script, OnDeath, OnUpdate);
	};


	if (game.skip_to_boss) {
		goto l_boss;
	}

	if (game.skip_to_midboss) {
		goto l_midboss;
	}


	wait(60);

#if 1
	{
		Repeat (50) {
			spawn_spinner();
			wait(5);
		}

		wait(180);

		for (int j = 1; j <= 4; j++) {
			for (int i = 1; i <= 18; i++) {
				float x = i * (PLAY_AREA_W / 19.0f);
				if (j % 2 == 0) {
					x = PLAY_AREA_W - x;
				}
				float y = 0;

				CreateEnemy(x, y, 2, point_direction(x, y, world.player.x, world.player.y), 0, spr_fairy_0, 10, 2);

				if (i >= 12) {
					spawn_spinner();
				}

				wait(20);
			}

			wait(60);
		}

		wait(180);
	}
#endif

l_midboss:

	{
		instance_id dai_chan = CreateBoss(BOSS_DAIYOUSEI_MIDBOSS)->id;
		while (world.find_boss(dai_chan)) {
			wait(1);
		}
	}

	wait(60);

l_boss:

	{
		instance_id baka = CreateBoss(BOSS_CIRNO)->id;
		// while (world.find_boss(baka)) {
		// 	wait(1);
		// }
	}

#if 0
	CreateBoss(MIDBOSS_YOUMU);
#endif
}


static u32 vao;
static u32 vbo;
static u32 ebo;
static u32 program;

static const u32 indices[] = {
	0, 1, 2,
	2, 3, 0,
};

static const char vert_src[] = R"(
#version 330 core

#ifdef GL_ES
precision mediump float;
#endif

layout(location = 0) in vec3 in_Position;
layout(location = 1) in vec3 in_Normal;
layout(location = 2) in vec4 in_Color;
layout(location = 3) in vec2 in_TexCoord;

out vec4 v_Color;
out vec3 v_WorldPos;
out vec3 v_MVPposition;

uniform mat4 u_Model;
uniform mat4 u_View;
uniform mat4 u_Proj;

void main() {
	mat4 MVP = (u_Proj * u_View) * u_Model;

	gl_Position = MVP * vec4(in_Position, 1.0);

	v_Color       = in_Color;
	v_WorldPos    = (u_Model * vec4(in_Position, 1.0)).xyz;
	v_MVPposition = gl_Position.xyz;
}
)";

static const char frag_src[] = R"(
#version 330 core

#ifdef GL_ES
precision mediump float;
#endif

layout(location = 0) out vec4 FragColor;

in vec4 v_Color;
in vec3 v_WorldPos;
in vec3 v_MVPposition;

uniform sampler2D u_Texture;
uniform float u_Time;


/* FOG */

uniform float u_FogStart;
uniform float u_FogEnd;
uniform vec4  u_FogColor;

vec4 apply_fog(vec4 color, vec3 MVPposition) {
	const vec3  FogOrigin = vec3(0.0, 0.0, 0.0);
	const float FogMax    = 0.95;

	float dist = length(MVPposition - FogOrigin);
	float fraction = clamp((dist - u_FogStart) / (u_FogEnd - u_FogStart), 0.0, FogMax);

	color = mix(color, u_FogColor, fraction);
	return color;
}

/* FOG */


void main() {
	vec2 texcoord1 = vec2(
		v_WorldPos.x * 0.05 + u_Time * 0.04,
		v_WorldPos.z * 0.05 - u_Time * 0.02
	);

	vec2 texcoord2 = vec2(
		v_WorldPos.x * 0.05 - u_Time * 0.04,
		v_WorldPos.z * 0.05 - u_Time * 0.02
	);

	vec4 color1 = texture(u_Texture, texcoord1);
	vec4 color2 = texture(u_Texture, texcoord2);

	vec4 color = mix(color1, color2, 0.5);

	color *= v_Color;

	color = apply_fog(color, v_MVPposition);

	FragColor = color;
	// FragColor = vec4(fract(v_Position), 1.0);
}
)";

void Stage_0_Init_Background() {
	world.d3d.cam_pos = {0, 10, 0};
	world.d3d.pitch   = -45;
	world.d3d.yaw     = -90;

	if (!vao) {
		Assert(!vbo);
		Assert(!ebo);

		float x1 = -100;
		float z1 = -100;
		float x2 = 100;
		float z2 = 100;

		float u1 = 0;
		float v1 = 0;
		float u2 = 1;
		float v2 = 1;

		Vertex vertices[] = {
			{{x1, 0, z1}, {}, color_white, {u1, v1}},
			{{x2, 0, z1}, {}, color_white, {u2, v1}},
			{{x2, 0, z2}, {}, color_white, {u2, v2}},
			{{x1, 0, z2}, {}, color_white, {u1, v2}},
		};

		// @Leak
		vao = create_vertex_array_obj(vertices, ArrayLength(vertices), indices, ArrayLength(indices), &vbo, &ebo);
	}

	if (!program) {
		u32 vert = compile_shader(GL_VERTEX_SHADER, vert_src);
		defer { glDeleteShader(vert); };

		u32 frag = compile_shader(GL_FRAGMENT_SHADER, frag_src);
		defer { glDeleteShader(frag); };

		// @Leak
		program = link_program(vert, frag);
	}
}

void Stage_0_Draw_Background(float delta) {
	break_batch();

	const vec4 fog_color = {1, 1, 1, 1};

	glClearColor(fog_color.r, fog_color.g, fog_color.b, fog_color.a);
	glClear(GL_COLOR_BUFFER_BIT);

	{
		glUseProgram(program);

		mat4 model = {1};
		mat4 view  = world.d3d.get_view_mat();
		mat4 proj  = world.d3d.get_proj_mat();

		int u_Model    = glGetUniformLocation(program, "u_Model");
		int u_View     = glGetUniformLocation(program, "u_View");
		int u_Proj     = glGetUniformLocation(program, "u_Proj");
		int u_Time     = glGetUniformLocation(program, "u_Time");
		int u_FogStart = glGetUniformLocation(program, "u_FogStart");
		int u_FogEnd   = glGetUniformLocation(program, "u_FogEnd");
		int u_FogColor = glGetUniformLocation(program, "u_FogColor");

		glUniformMatrix4fv(u_Model, 1, GL_FALSE, &model[0][0]);
		glUniformMatrix4fv(u_View,  1, GL_FALSE, &view[0][0]);
		glUniformMatrix4fv(u_Proj,  1, GL_FALSE, &proj[0][0]);
		glUniform1f(u_Time, SDL_GetTicks() / 1000.0f);
		glUniform1f(u_FogStart, 20);
		glUniform1f(u_FogEnd, 60);
		glUniform4fv(u_FogColor, 1, &fog_color[0]);

		glBindTexture(GL_TEXTURE_2D, get_texture(tex_eosd_misty_lake).id);
		glBindVertexArray(vao);

		glDrawElements(GL_TRIANGLES, ArrayLength(indices), GL_UNSIGNED_INT, 0);
		renderer.curr_draw_calls++;

		glBindVertexArray(0);
		glBindTexture(GL_TEXTURE_2D, 0);

		glUseProgram(0);
	}
}
