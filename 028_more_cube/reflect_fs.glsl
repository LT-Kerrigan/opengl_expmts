#version 410

in vec3 texcoords;
uniform samplerCube cube_texture;
uniform vec3 light_pos_wor;
out vec4 frag_colour;

float shad (float l, vec3 tc) {
	float texel = texture (cube_texture, tc).r;
	// bias is used in case of self-shadowing
	float bias = 0.025;
	if (texel + bias < l) {
		return 0.5;
	}
	return 1.0;
}

void main () {
	vec3 base = vec3 (0.1, 0.3, 0.3);
	frag_colour = vec4 (base, 1.0);

	// note: texcoords are also world position of cube verts/frags
	float l = length (texcoords - light_pos_wor);

//	{ // shadow cube single sample version
//		frag_colour.rgb *= shad (l, texcoords);
//	}
	{ // blurred v
		float per = 0.0125;
		float a = shad (l, texcoords + vec3 (per, 0.0, 0.0));
		float b = shad (l, texcoords + vec3 (-per, 0.0, 0.0));
		float c = shad (l, texcoords + vec3 (0.0, per, 0.0));
		float d = shad (l, texcoords + vec3 (0.0, -per, 0.0));
		float e = shad (l, texcoords + vec3 (0.0, 0.0, per));
		float f = shad (l, texcoords + vec3 (0.0, 0.0, -per));
		float r = (a + b + c + d + e + f) / 6.0;
		//r = r * 0.5 + 0.5;
		frag_colour.rgb *= r;
	}
}