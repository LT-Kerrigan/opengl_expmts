#version 410

in vec3 dist_of_light;
uniform samplerCube depth_map;
uniform vec3 colour;
out vec4 frag_colour;

float eval_shadow () {
	vec3 v = (dist_of_light);
	float cubes_dist = textureCube (depth_map, v).r;
	float dist_wor = length (v);

	float n = 1.0f;
	float f = 50.0f;
	float z = 2.0 * n * f / (f + n - cubes_dist * (f - n));
	
	float dn = 2.0 * z - 1.0;
	dn = (dn - n) / (f - n);
	dist_wor = (dist_wor - n) / (f - n);
	
	float epsilon = -0.5;
	if (dist_wor < dn + epsilon) {
		return 1.0;
	} else {
		return 0.5;
	}
}

void main() {
	frag_colour = vec4 (colour, 1.0);
	float shadow_factor = eval_shadow ();
	frag_colour = vec4 (colour * shadow_factor, 1.0);
}
