#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <stdbool.h>

#define MAX_SHADERS 128

struct Shader {
	GLuint sp, vs, fs, gs, tcs, tes, cs;
	bool all_compiled;
	bool linked;
	bool has_M, has_V, has_P, has_MVP, has_MV, has_VP, has_cam_pos;
	int M_loc, V_loc, P_loc, MVP_loc, MV_loc, VP_loc, cam_pos_loc;
};

struct Shaders {
	Shader shaders[128];
	int shader_count;
	// TODO plus default shaders
	// uses game-specific shader index to identify things
	// resolves to crappy GL indices internally
};

bool init_shaders ();
bool create_shader_from_files (const char* vs_fn, const char* fs_fn);
bool uniform_M (int spi, mat4 M);
bool uniform_V (int spi, mat4 M);
bool uniform_P (int spi, mat4 M);
bool uniform_cam_pos (int spi, vec3 v);
