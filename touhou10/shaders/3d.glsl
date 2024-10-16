#line 2

#ifdef VERTEX_SHADER

layout(location = 0) in vec3 in_Position;
layout(location = 1) in vec3 in_Normal;
layout(location = 2) in vec4 in_Color;
layout(location = 3) in vec2 in_TexCoord;

out vec4 v_Color;
out vec2 v_TexCoord;
out vec3 v_MVPposition;

uniform mat4 u_Model;
uniform mat4 u_View;
uniform mat4 u_Proj;

void main() {
	mat4 MVP = (u_Proj * u_View) * u_Model;

	gl_Position = MVP * vec4(in_Position, 1.0);

	vec4 color = in_Color;

	color = apply_directional_light(color, u_Model, in_Normal);

	v_Color       = color;
	v_TexCoord    = in_TexCoord;
	v_MVPposition = gl_Position.xyz;
}

#endif



#ifdef FRAGMENT_SHADER

#ifdef GL_ES
precision mediump float;
#endif

layout(location = 0) out vec4 FragColor;

in vec4 v_Color;
in vec2 v_TexCoord;
in vec3 v_MVPposition;

uniform sampler2D u_Texture;

void main() {
	vec4 color = texture(u_Texture, v_TexCoord);

	color *= v_Color;

	color = apply_fog(color, v_MVPposition);

	FragColor = color;
}

#endif
