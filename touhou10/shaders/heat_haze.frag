layout(location = 0) out vec4 FragColor;

in vec4 v_Color;
in vec2 v_TexCoord;
in vec3 v_EyePos;

uniform sampler2D u_Texture;
uniform sampler2D u_DistortionTexture;
uniform float u_Time;
uniform vec2 u_Position;

const vec2 u_Size = vec2(0.15, 0.15);
const float u_DistortionStrength = 0.05;
const float u_Radius = 100.0;

void main() {
	float dist = distance(v_EyePos.xy, u_Position);

	float factor = (1.0 - min(dist / u_Radius, 1.0));
	// factor = dist < u_Radius ? 1.0 : 0.0;

	float strength = u_DistortionStrength * factor;

	vec2 distort;
	distort.x = texture(u_DistortionTexture, v_TexCoord * u_Size       + vec2(0.0, u_Time)).r       * strength;
	distort.y = texture(u_DistortionTexture, v_TexCoord * u_Size * 3.4 + vec2(0.0, u_Time * 1.6)).g * strength * 1.3;

	vec4 color = texture(u_Texture, v_TexCoord + distort);
	color.rgb = mix(color.rgb, color.rgb * 0.6, factor);

	FragColor = color * v_Color;
}
