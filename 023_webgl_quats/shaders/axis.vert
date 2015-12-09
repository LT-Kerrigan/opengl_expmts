//
// axis line shader
// Anton Gerdelan 9 Dec 2015
// antongerdelan.net
//

attribute vec3 vp; // points
attribute vec3 vc; // colours

uniform mat4 M, V, P;

varying vec3 c;

void main () {
	gl_Position = P * V * vec4 (vp, 1.0);
	c = vc;
}
