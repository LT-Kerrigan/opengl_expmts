#version 410

in vec2 st;
uniform samplerCube depth_tex;
out vec4 frag_colour;

void main () {
	float d = textureCube (depth_tex, normalize (vec3 (
		2.0 * st.s - 1.0,
		-1.0,
		-2.0 * st.t + 1.0
	))).r;
	frag_colour = vec4 (d, d, d, 1.0);
}
