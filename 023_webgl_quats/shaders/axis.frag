//
// axis line shader
// Anton Gerdelan 9 Dec 2015
// antongerdelan.net
//

precision mediump float;

varying vec3 c;

void main () {
	gl_FragColor = vec4 (c, 1.0);
}
