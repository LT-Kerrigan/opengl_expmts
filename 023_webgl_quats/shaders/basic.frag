//
// basic texturing shader
// Anton Gerdelan 23 Dec 2014
// antongerdelan.net
//

precision mediump float;

varying vec2 st;
varying vec3 n, p;
//uniform sampler2D dm;

void main () {
	//
	// shading darker based on angle to eye
	vec3 n_eye = normalize (n);
	vec3 p_to_eye = normalize (-p);
	float dp = dot (n_eye, p_to_eye);
	
	//vec4 texel = texture2D (dm, st);
	
	//gl_FragColor = vec4 (texel.rgb * dp, 1.0);
	gl_FragColor = vec4 (vec3 (0.7, 0.7, 0.0) * dp, 1.0);
}
