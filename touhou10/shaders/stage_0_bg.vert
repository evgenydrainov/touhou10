#version 330 core

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
