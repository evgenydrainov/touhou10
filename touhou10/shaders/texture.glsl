#line 2

#ifdef VERTEX_SHADER

layout(location = 0) in vec3 in_Position;
layout(location = 1) in vec3 in_Normal;
layout(location = 2) in vec4 in_Color;
layout(location = 3) in vec2 in_TexCoord;

out vec4 v_Color;
out vec2 v_TexCoord;

uniform mat4 u_MVP;

void main() {
	gl_Position = u_MVP * vec4(in_Position, 1.0);

	v_Color    = in_Color;
	v_TexCoord = in_TexCoord;
}

#endif



#ifdef FRAGMENT_SHADER

layout(location = 0) out vec4 FragColor;

in vec4 v_Color;
in vec2 v_TexCoord;

uniform sampler2D u_Texture;

void main() {
	vec4 color = texture(u_Texture, v_TexCoord);

	FragColor = color * v_Color;
}

#endif
