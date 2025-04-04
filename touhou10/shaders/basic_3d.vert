layout(location = 0) in vec3 in_Position;
layout(location = 1) in vec3 in_Normal;
layout(location = 2) in vec4 in_Color;
layout(location = 3) in vec2 in_TexCoord;

out vec4 v_Color;
out vec2 v_TexCoord;
out vec3 v_EyePos;

uniform mat4 u_MVP;
uniform mat4 u_ModelView;

void main() {
	gl_Position = u_MVP * vec4(in_Position, 1.0);

	v_Color = in_Color;
	v_TexCoord = in_TexCoord;
	v_EyePos = (u_ModelView * vec4(in_Position, 1.0)).xyz;
}
