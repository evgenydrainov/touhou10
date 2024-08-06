#version 330 core

layout(location = 0) out vec4 FragColor;

in vec4 v_Color;
in vec2 v_TexCoord;
in vec3 v_WorldPos;

uniform sampler2D u_Texture;

uniform float u_FogStart;
uniform float u_FogEnd;
uniform vec4  u_FogColor;

const vec3  FogOrigin = vec3(0.0, 0.0, 0.0);
const float FogMax    = 0.95;

// 
// Stolen from DragoniteSpam
// 

void main() {
	vec4 color = texture(u_Texture, v_TexCoord);

	// apply before fog?
	color *= v_Color;

	// Apply fog
	{
		float dist = length(v_WorldPos - FogOrigin);
		float fraction = clamp((dist - u_FogStart) / (u_FogEnd - u_FogStart), 0.0, FogMax);

		color = mix(color, u_FogColor, fraction);
	}

	FragColor = color;
}
