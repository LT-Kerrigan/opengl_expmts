#include "shader.h"

Shaders g_shaders;

bool init_shaders () {
	char* vs[1] = {
		"basic.vert"
	};
	char* fs[1] = {
		"basic.frag"
	};
	// TODO create default shader
	for (int i = 0; i < 1; i++) {
		// TODO create shader from files
		// if (!create_shader_from_files (vs[i], fs[i])) {
		//
		//}
	}
	
	return true;
}

bool create_shader_from_files (const char* vs_fn, const char* fs_fn) {
	return true;
}

bool uniform_M (int spi, mat4 M) {
	// TODO check if shader exists
	// TODO check if compiled and linked
	// TODO if not try to use default shader instead
	// TODO check if approp shader is in use
	// TODO if not switch to it
	// TODO do the uniform update
}

bool uniform_V (int spi, mat4 M);
bool uniform_P (int spi, mat4 M);
bool uniform_cam_pos (int spi, vec3 v);
