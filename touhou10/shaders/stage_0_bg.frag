#version 330 core

layout(location = 0) out vec4 FragColor;

in vec4 v_Color;
in vec3 v_WorldPos;
in vec3 v_MVPposition;

uniform sampler2D u_Texture;
uniform float u_Time;

// @Copy
uniform float u_FogStart;
uniform float u_FogEnd;
uniform vec4  u_FogColor;

const vec3  FogOrigin = vec3(0.0, 0.0, 0.0);
const float FogMax    = 0.95;

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

	// Apply fog
	{
		float dist = length(v_MVPposition - FogOrigin);
		float fraction = clamp((dist - u_FogStart) / (u_FogEnd - u_FogStart), 0.0, FogMax);

		color = mix(color, u_FogColor, fraction);
	}

	FragColor = color;
	// FragColor = vec4(fract(v_Position), 1.0);
}
