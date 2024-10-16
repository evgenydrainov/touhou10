#line 2

/*
	Author: rsn8887 (based on TheMaister)
	License: Public domain

	This is an integer prescale filter that should be combined
	with a bilinear hardware filtering (GL_BILINEAR filter or some such) to achieve
	a smooth scaling result with minimum blur. This is good for pixelgraphics
	that are scaled by non-integer factors.
*/

#ifdef FRAGMENT_SHADER

#ifdef GL_ES
precision mediump float;
#endif

layout(location = 0) out vec4 FragColor;

in vec4 v_Color;
in vec2 v_TexCoord;

uniform sampler2D u_Texture;

uniform vec2 u_SourceSize;
uniform vec2 u_Scale; // The integer scale.

void main() {
	vec2 texel = v_TexCoord * u_SourceSize;
	vec2 scale = u_Scale;

	vec2 texel_floored = floor(texel);
	vec2 s = fract(texel);
	vec2 region_range = 0.5 - 0.5 / scale;

	// Figure out where in the texel to sample to get correct pre-scaled bilinear.
	// Uses the hardware bilinear interpolator to avoid having to sample 4 times manually.

	vec2 center_dist = s - 0.5;
	vec2 f = (center_dist - clamp(center_dist, -region_range, region_range)) * scale + 0.5;

	vec2 mod_texel = texel_floored + f;

	vec4 color = texture(u_Texture, mod_texel / u_SourceSize);
	FragColor = color * v_Color;
}

#endif
