#ifdef VERTEX_SHADER

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

#endif



#ifdef FRAGMENT_SHADER

layout(location = 0) out vec4 FragColor;

in vec4 v_Color;
in vec3 v_WorldPos;
in vec3 v_MVPposition;

uniform sampler2D u_Texture;
uniform float u_Time;

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

#endif
