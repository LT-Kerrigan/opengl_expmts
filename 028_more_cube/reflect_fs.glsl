#version 410

in vec3 texcoords;
uniform samplerCube cube_texture;
uniform vec3 light_pos_wor;
out vec4 frag_colour;

void main () {
	vec3 base = vec3 (0.1, 0.3, 0.3);
	frag_colour = vec4 (base, 1.0);

	{ // shadow cube
		float texel = texture (cube_texture, texcoords).r;
		// note: texcoords are also world position of cube verts/frags

		// TODO for self-shadows
		// if length is CLOSER than own 1d distance to light -->shadow, else not
		// and add bias
		float bias = 0.025;
		float l = length (texcoords - light_pos_wor);
		if (texel + bias < l) {
			frag_colour.rgb *= 0.2;
		}

	}
}
