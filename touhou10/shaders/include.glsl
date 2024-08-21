
// 
// Stolen from DragoniteSpam
// 

#ifdef VERTEX_SHADER

uniform vec3 u_LightDirection; // @Todo: don't pay for what you don't use
// uniform vec4 u_LightColor;

vec4 apply_directional_light(vec4 color, mat4 model, vec3 normal) {
	// vec4 lightAmbient = vec4(0.0, 0.0, 0.0, 1.0);
	vec3 lightDir = normalize(-u_LightDirection);
	vec3 worldNormal = normalize(model * vec4(normal, 0.0)).xyz;

	float lightAngleDifference = max(dot(worldNormal, lightDir), 0.0);

	// color.rgb *= min(lightAmbient + u_LightColor * lightAngleDifference, vec4(1.0)).rgb;

	float low = 0.5;
	color.rgb *= low + ((1.0 - low) * lightAngleDifference);
	return color;
}

#endif



#ifdef FRAGMENT_SHADER

uniform float u_FogStart;   // @Todo: don't pay for what you don't use
uniform float u_FogEnd;
uniform vec4  u_FogColor;

vec4 apply_fog(vec4 color, vec3 MVPposition) {
	const vec3  FogOrigin = vec3(0.0, 0.0, 0.0);
	const float FogMax    = 0.95;

	float dist = length(MVPposition - FogOrigin);
	float fraction = clamp((dist - u_FogStart) / (u_FogEnd - u_FogStart), 0.0, FogMax);

	color = mix(color, u_FogColor, fraction);
	return color;
}

#endif

