#version 410

in vec3 pos_eye;
in vec3 n_eye;
uniform samplerCube cube_texture;
uniform mat4 V; // view matrix
out vec4 frag_colour;

void main () {
	/* reflect ray around normal from eye to surface */
	vec3 incident_eye = normalize (pos_eye);
	vec3 normal = normalize (n_eye);

	vec3 reflected = reflect (incident_eye, normal);
	// convert from eye to world space
	reflected = vec3 (inverse (V) * vec4 (reflected, 0.0));

	vec3 texel = texture (cube_texture, reflected).rgb;
	vec3 base = vec3 (0.1, 0.3, 0.3);
	//vec3 c = mix (texel, base, 0.9);
	frag_colour = vec4 (base, 1.0);
}
