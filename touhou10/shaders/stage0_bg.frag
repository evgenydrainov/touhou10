layout(location = 0) out vec4 FragColor;

in vec4 v_Color;
in vec2 v_TexCoord;
in vec3 v_MVPposition;

uniform sampler2D u_Texture;
uniform vec2 u_Offset;
uniform vec4 u_Color;


/* FOG */

uniform float u_FogStart;
uniform float u_FogEnd;
uniform vec4  u_FogColor;

vec4 apply_fog(vec4 color, vec3 MVPposition) {
	const vec3  FogOrigin = vec3(0.0, 0.0, 0.0);
	const float FogMax    = 0.95;

	float dist = length(MVPposition - FogOrigin);
	float fraction = clamp((dist - u_FogStart) / (u_FogEnd - u_FogStart), 0.0, FogMax);

	color.rgb = mix(color.rgb, u_FogColor.rgb, fraction);
	return color;
}

/* FOG */


void main() {
	vec4 color = texture(u_Texture, v_TexCoord + u_Offset);
	color *= v_Color;
	color *= u_Color;

	color = apply_fog(color, v_MVPposition);

	FragColor = color;
	// FragColor = vec4(fract(v_Position), 1.0);
}
