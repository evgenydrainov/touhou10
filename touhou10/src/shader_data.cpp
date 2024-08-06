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


const char shader_sharp_bilinear_fragment_text[] = R"END_SHADER(
/*
	Author: rsn8887 (based on TheMaister)
	License: Public domain

	This is an integer prescale filter that should be combined
	with a bilinear hardware filtering (GL_BILINEAR filter or some such) to achieve
	a smooth scaling result with minimum blur. This is good for pixelgraphics
	that are scaled by non-integer factors.
*/

#version 330 core

layout(location = 0) out vec4 FragColor;

in vec4 v_Color;
in vec2 v_TexCoord;

uniform sampler2D u_Texture;

uniform vec2 u_SourceSize;
uniform vec2 u_Scale; // The integer scale.

void main() {
	vec2 texel = v_TexCoord * u_SourceSize;
	vec2 scale = u_Scale;

	vec2 texel_floored = floor(texel);
	vec2 s = fract(texel);
	vec2 region_range = 0.5 - 0.5 / scale;

	// Figure out where in the texel to sample to get correct pre-scaled bilinear.
	// Uses the hardware bilinear interpolator to avoid having to sample 4 times manually.

	vec2 center_dist = s - 0.5;
	vec2 f = (center_dist - clamp(center_dist, -region_range, region_range)) * scale + 0.5;

	vec2 mod_texel = texel_floored + f;

	vec4 color = texture(u_Texture, mod_texel / u_SourceSize);
	FragColor = color * v_Color;
}
)END_SHADER";


const char shader_3d_vertex_text[] = R"END_SHADER(
#version 330 core

layout(location = 0) in vec3 in_Position;
layout(location = 1) in vec4 in_Color;
layout(location = 2) in vec2 in_TexCoord;

out vec4 v_Color;
out vec2 v_TexCoord;
out vec3 v_WorldPos;

uniform mat4 u_MVP;

uniform vec3 u_LightDirection;
uniform vec4 u_LightColor;

// 
// Stolen from DragoniteSpam
// 

void main() {
	gl_Position = u_MVP * vec4(in_Position, 1.0);

	vec4 color = in_Color;

	// Apply directional light
	/*{
		vec4 lightAmbient = vec4(0.25, 0.25, 0.25, 1.0);
		vec3 lightDir = normalize(-u_LightDirection);
		vec3 worldNormal = normalize(u_MVP * vec4(in_Normal, 0.0)).xyz;

		float lightAngleDifference = max(dot(worldNormal, lightDir), 0.0);

		color.rgb *= min(lightAmbient + u_LightColor * lightAngleDifference, vec4(1.0)).rgb;
	}*/

	v_Color    = color;
	v_TexCoord = in_TexCoord;
	v_WorldPos = gl_Position.xyz;
}

)END_SHADER";


const char shader_3d_fragment_text[] = R"END_SHADER(
#version 330 core

layout(location = 0) out vec4 FragColor;

in vec4 v_Color;
in vec2 v_TexCoord;
in vec3 v_WorldPos;

uniform sampler2D u_Texture;

uniform float u_FogStart;
uniform float u_FogEnd;
uniform vec4  u_FogColor;

const vec3  FogOrigin   = vec3(0.0, 0.0, 0.0);
const float FogStrength = 0.9;

// 
// Stolen from DragoniteSpam
// 

void main() {
	vec4 color = texture(u_Texture, v_TexCoord);

	// Apply fog
	{
		float dist = length(v_WorldPos - FogOrigin);
		float fraction = clamp((dist - u_FogStart) / (u_FogEnd - u_FogStart), 0.0, 1.0);

		color = mix(color, u_FogColor, fraction * FogStrength);
	}

	FragColor = color * v_Color;
}

)END_SHADER";

