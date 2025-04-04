layout(location = 0) out vec4 FragColor;

in vec4 v_Color;
in vec2 v_TexCoord;
in vec3 v_EyePos;

uniform sampler2D u_Texture;

uniform vec4 u_FogColor;
uniform float u_FogStart;
uniform float u_FogEnd; 
uniform float u_FogDensity; 

vec4 apply_fog(vec4 color) {
	float c = length(v_EyePos);

	// linear
	// float fog_factor = (u_FogEnd - c) / (u_FogEnd - u_FogStart);

	// exp
	// float fog_factor = exp(-(4.0 * c / u_FogEnd) * u_FogDensity);

	// exp2
	float fog_factor = exp(-(4.0 * c / u_FogEnd) * u_FogDensity * (4.0 * c / u_FogEnd) * u_FogDensity);

	fog_factor = clamp(fog_factor, 0.0, 1.0);

	color.rgb = mix(u_FogColor.rgb, color.rgb, fog_factor);
	return color;
}

void main() {
	vec4 color = texture(u_Texture, v_TexCoord) * v_Color;
	color = apply_fog(color);

	FragColor = color;
}
