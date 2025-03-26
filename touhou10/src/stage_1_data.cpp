#include "scripting.h"

#include "renderer.h"
#include "util.h"

void Stage_1_Script(mco_coro* co) {
	wait(60);

	CreateBoss(BOSS_YOUMU_MIDBOSS);
}


static void draw_world_origin_axis(float delta) {
	static u32 vao;

	const float size = 10;
	static const Vertex vertices[] = {
		{{0,    0, 0}, {}, color_red},
		{{size, 0, 0}, {}, color_red},

		{{0, 0,    0}, {}, color_green},
		{{0, size, 0}, {}, color_green},

		{{0, 0, 0},    {}, color_blue},
		{{0, 0, size}, {}, color_blue},
	};

	if (!vao) {
		// @Leak
		vao = create_vertex_array_obj(vertices, ArrayLength(vertices));
	}

	u32 program = renderer.texture_shader.id;

	glUseProgram(program);
	defer { glUseProgram(0); };

	mat4 MVP = world.cam3d_get_mvp();

	int u_MVP = glGetUniformLocation(program, "u_MVP");
	glUniformMatrix4fv(u_MVP, 1, GL_FALSE, &MVP[0][0]);

	glBindTexture(GL_TEXTURE_2D, renderer.texture_for_shapes.id);
	defer { glBindTexture(GL_TEXTURE_2D, 0); };

	glBindVertexArray(vao);
	defer { glBindVertexArray(0); };

	glEnable(GL_DEPTH_TEST);
	defer { glDisable(GL_DEPTH_TEST); };

	glDrawArrays(GL_LINES, 0, ArrayLength(vertices));
	renderer.curr_draw_calls++;
}


static const char vert_src[] =
R"(layout(location = 0) in vec3 in_Position;
layout(location = 1) in vec3 in_Normal;
layout(location = 2) in vec4 in_Color;
layout(location = 3) in vec2 in_TexCoord;

out vec4 v_Color;
out vec2 v_TexCoord;
out vec3 v_MVPposition;

uniform mat4 u_Model;
uniform mat4 u_View;
uniform mat4 u_Proj;


/* DIRECTIONAL LIGHTING */

uniform vec3 u_LightDirection;
// uniform vec4 u_LightColor;

vec4 apply_directional_light(vec4 color, mat4 model, vec3 normal) {
	// vec4 lightAmbient = vec4(0.0, 0.0, 0.0, 1.0);
	vec3 lightDir = normalize(-u_LightDirection);
	vec3 worldNormal = normalize(model * vec4(normal, 0.0)).xyz;

	float lightAngleDifference = max(dot(worldNormal, lightDir), 0.0);

	// color.rgb *= min(lightAmbient + u_LightColor * lightAngleDifference, vec4(1.0)).rgb;

	float low = 0.5;
	color.rgb *= low + ((1.0 - low) * lightAngleDifference);
	return color;
}

/* DIRECTIONAL LIGHTING */


void main() {
	mat4 MVP = (u_Proj * u_View) * u_Model;

	gl_Position = MVP * vec4(in_Position, 1.0);

	vec4 color = in_Color;

	color = apply_directional_light(color, u_Model, in_Normal);

	v_Color       = color;
	v_TexCoord    = in_TexCoord;
	v_MVPposition = gl_Position.xyz;
}
)";

static const char frag_src[] =
R"(layout(location = 0) out vec4 FragColor;

in vec4 v_Color;
in vec2 v_TexCoord;
in vec3 v_MVPposition;

uniform sampler2D u_Texture;


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
	vec4 color = texture(u_Texture, v_TexCoord);

	color *= v_Color;

	color = apply_fog(color, v_MVPposition);

	FragColor = color;
}
)";

static u32 vao;
static int num_vertices;
static u32 program;

void Stage_1_Init_Background() {
	world.cam3d.pos = {0, 10.0f, 10.0f};
	world.cam3d.pitch   = -10;
	world.cam3d.yaw     = -90;

	if (!vao) {
		// @Leak
		vao = load_3d_model_from_obj_file("models/pcb_youmu_stairs.obj", &num_vertices);
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

void Stage_1_Draw_Background(float delta) {
	const vec4 fog_color = get_color(0xB08190FFu);

	break_batch();

	glClearColor(fog_color.r, fog_color.g, fog_color.b, fog_color.a);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	if (SDL_GetRelativeMouseMode()) {
		draw_world_origin_axis(delta);
	}

	{
		glUseProgram(program);
		defer { glUseProgram(0); };

		float y = -wrapf(SDL_GetTicks() / 1000.0f, 8.0f);
		float z =  wrapf(SDL_GetTicks() / 1000.0f, 8.0f);

		mat4 model = glm::translate(mat4{1}, vec3{0, y, z});
		mat4 view  = world.cam3d_get_view_mat();
		mat4 proj  = world.cam3d_get_proj_mat();

		int u_Model          = glGetUniformLocation(program, "u_Model");
		int u_View           = glGetUniformLocation(program, "u_View");
		int u_Proj           = glGetUniformLocation(program, "u_Proj");
		int u_FogStart       = glGetUniformLocation(program, "u_FogStart");
		int u_FogEnd         = glGetUniformLocation(program, "u_FogEnd");
		int u_FogColor       = glGetUniformLocation(program, "u_FogColor");
		int u_LightDirection = glGetUniformLocation(program, "u_LightDirection");

		glUniformMatrix4fv(u_Model, 1, GL_FALSE, &model[0][0]);
		glUniformMatrix4fv(u_View,  1, GL_FALSE, &view[0][0]);
		glUniformMatrix4fv(u_Proj,  1, GL_FALSE, &proj[0][0]);
		glUniform1f(u_FogStart, 20);
		glUniform1f(u_FogEnd, 40);
		glUniform4fv(u_FogColor, 1, &fog_color[0]);
		glUniform3f(u_LightDirection, 0, -1, 0);

		glBindTexture(GL_TEXTURE_2D, get_texture(tex_pcb_youmu_stairs).id);
		defer { glBindTexture(GL_TEXTURE_2D, 0); };

		glBindVertexArray(vao);
		defer { glBindVertexArray(0); };

		glEnable(GL_DEPTH_TEST);
		defer { glDisable(GL_DEPTH_TEST); };

		glEnable(GL_CULL_FACE);
		defer { glDisable(GL_CULL_FACE); };

		glDrawArrays(GL_TRIANGLES, 0, num_vertices);
		renderer.curr_draw_calls++;
	}
}
