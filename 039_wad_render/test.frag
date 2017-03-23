#version 120

varying vec3 n_eye;

void main() {
	vec3 nn = normalize(n_eye);
	float fac = max(0.0, dot(nn, vec3(0.0,0.0,1.0)));
	gl_FragColor = vec4 (1.0, 1.0, 1.0, 1.0);
	gl_FragColor.rgb = vec3(fac * 0.5 + 0.5);
}
