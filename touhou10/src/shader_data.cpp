#include "assets.h"


const char shader_texture_vertex_text[] = R"END_SHADER(
#version 330 core

layout(location = 0) in vec3 in_Position;
layout(location = 1) in vec4 in_Color;
layout(location = 2) in vec2 in_TexCoord;

out vec4 v_Color;
out vec2 v_TexCoord;

uniform mat4 u_MVP;

void main() {
	gl_Position = u_MVP * vec4(in_Position, 1.0);

	v_Color = in_Color;
	v_TexCoord = in_TexCoord;
}

)END_SHADER";


const char shader_texture_fragment_text[] = R"END_SHADER(
#version 330 core

layout(location = 0) out vec4 FragColor;

in vec4 v_Color;
in vec2 v_TexCoord;

uniform sampler2D u_Texture;

void main() {
	vec4 color = texture(u_Texture, v_TexCoord);
	FragColor = color * v_Color;
	// FragColor = vec4(1.0);
}

)END_SHADER";


const char shader_color_fragment_text[] = R"END_SHADER(
#version 330 core

layout(location = 0) out vec4 FragColor;

in vec4 v_Color;
in vec2 v_TexCoord;

void main() {
	FragColor = v_Color;
}

)END_SHADER";


const char shader_stage_0_bg_vertex_text[] = R"END_SHADER(
#version 330 core

layout(location = 0) in vec3 in_Position;
layout(location = 1) in vec4 in_Color;
layout(location = 2) in vec2 in_TexCoord;

out vec4 v_Color;
out vec3 v_Position;

uniform mat4 u_MVP;

void main() {
	gl_Position = u_MVP * vec4(in_Position, 1.0);

	v_Color = in_Color;
	v_Position = in_Position;
}

)END_SHADER";


const char shader_stage_0_bg_fragment_text[] = R"END_SHADER(
#version 330 core

layout(location = 0) out vec4 FragColor;

in vec4 v_Color;
in vec3 v_Position;

uniform sampler2D u_Texture;
uniform float u_Time;

void main() {
	vec2 texcoord1 = vec2(
		fract(v_Position.x * 0.1 + u_Time * 0.04),
		fract(v_Position.z * 0.1 - u_Time * 0.02)
	);

	vec2 texcoord2 = vec2(
		fract(v_Position.x * 0.1 - u_Time * 0.04),
		fract(v_Position.z * 0.1 - u_Time * 0.02)
	);

	vec4 color1 = texture(u_Texture, texcoord1);
	vec4 color2 = texture(u_Texture, texcoord2);

	vec4 color = mix(color1, color2, 0.5);

	FragColor = color * v_Color;
	// FragColor = vec4(fract(v_Position), 1.0);
}

)END_SHADER";


#if 0
const char shader_particle_vertex_text[] = R"END_SHADER(
#version 330 core

layout(location = 0) in vec3 in_Position;
layout(location = 1) in vec4 in_Color;
layout(location = 2) in vec2 in_TexCoord;

// in_Position.xy : vec2  init_pos
// in_Color.xy    : vec2  init_spd
// in_Color.z     : float acc
// in_Color.w     : float time

out vec4 v_Color;
out vec2 v_TexCoord;

uniform mat4 u_MVP;

void main() {
	vec2  init_pos = in_Position.xy;
	vec2  init_spd = in_Color.xy;
	float acc      = in_Color.z;
	float time     = in_Color.w;

	vec2 position = init_pos + init_spd * time + vec2(acc, 0.0) * time * time / 2.0;

	gl_Position = u_MVP * vec4(position, 0.0, 1.0);

	v_Color = vec4(1.0);
	v_TexCoord = in_TexCoord;
}

)END_SHADER";


const char shader_particle_fragment_text[] = R"END_SHADER(
#version 330 core

layout(location = 0) out vec4 FragColor;

in vec4 v_Color;
in vec2 v_TexCoord;

uniform sampler2D u_Texture;

void main() {
	vec4 color = texture(u_Texture, v_TexCoord);
	FragColor = color * v_Color;
}

)END_SHADER";
#endif
