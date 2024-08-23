#line 2

#ifdef FRAGMENT_SHADER

layout(location = 0) out vec4 FragColor;

in vec4 v_Color;
in vec2 v_TexCoord;

void main() {
	FragColor = v_Color;
}

#endif
