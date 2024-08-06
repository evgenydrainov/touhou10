#version 330 core

layout(location = 0) in vec3 in_Position;
layout(location = 1) in vec3 in_Normal;
layout(location = 2) in vec4 in_Color;
layout(location = 3) in vec2 in_TexCoord;

out vec4 v_Color;
out vec2 v_TexCoord;
out vec3 v_WorldPos;

uniform mat4 u_Model;
uniform mat4 u_View;
uniform mat4 u_Proj;

uniform vec3 u_LightDirection;
// uniform vec4 u_LightColor;

// 
// Stolen from DragoniteSpam
// 

void main() {
	mat4 MVP = (u_Proj * u_View) * u_Model;

	gl_Position = MVP * vec4(in_Position, 1.0);

	vec4 color = in_Color;

	// Apply directional light
	{
		// vec4 lightAmbient = vec4(0.0, 0.0, 0.0, 1.0);
		vec3 lightDir = normalize(-u_LightDirection);
		vec3 worldNormal = normalize(u_Model * vec4(in_Normal, 0.0)).xyz;

		float lightAngleDifference = max(dot(worldNormal, lightDir), 0.0);

		// color.rgb *= min(lightAmbient + u_LightColor * lightAngleDifference, vec4(1.0)).rgb;

		float low = 0.5;
		color.rgb *= low + ((1.0 - low) * lightAngleDifference);
	}

	v_Color    = color;
	v_TexCoord = in_TexCoord;
	v_WorldPos = gl_Position.xyz;
}
