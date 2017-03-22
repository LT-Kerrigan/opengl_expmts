#version 120

varying float dist;

void main() {
	gl_FragColor = vec4 (1.0, 0.0, 0.0, 1.0);
	// use z position to shader darker to help perception of distance
	gl_FragColor.xyz *= dist;
}
