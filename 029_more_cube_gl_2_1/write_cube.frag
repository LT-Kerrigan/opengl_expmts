#version 120

varying vec3 dist_wor;
//out float frag_colour;

void main () {
	float l = length (dist_wor);
	//frag_colour = l;
	gl_FragColor = vec4 (l, l, l, 1.0);
}
