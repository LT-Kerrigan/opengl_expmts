// http://clang.llvm.org/docs/LanguageExtensions.html#vectors-and-extended-vectors
#include <stdio.h>

typedef float vec4 __attribute__((ext_vector_type(4)));
typedef float vec3 __attribute__((ext_vector_type(3)));

vec4 foo (vec3 a, float b) {
	vec4 c;
	c.xyz = a;
	c.w = b;
	return c;
}

int main () {
	vec4 a = {1.0,2.0,3.0,4.0};
	printf ("vector ext test\n");
	a = foo ((vec3){5.0f,6.0f, 7.0f}, 8.0f);
	printf ("%f %f %f %f\n", a.x, a.y, a.z, a.w);
}
