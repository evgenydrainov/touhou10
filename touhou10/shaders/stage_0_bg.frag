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
