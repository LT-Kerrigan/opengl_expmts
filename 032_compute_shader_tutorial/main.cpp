// compute shaders tutorial
// Dr Anton Gerdelan <gerdela@scss.tcd.ie>
// Trinity College Dublin, Ireland
// 26 Feb 2016

#include "gl_utils.h"

// this is the compute shader in an ugly C string
const char* compute_shader_str =
"#version 430\n                                                               \
layout (local_size_x = 1, local_size_y = 1) in;\n                             \
layout (rgba32f, binding = 0) uniform image2D img_output;\n                   \
\n                                                                            \
void main () {\n                                                              \
  vec4 texel = vec4 (0.0, 0.0, 0.0, 1.0);\n                                   \
  ivec2 p = ivec2 (gl_GlobalInvocationID.xy);\n                               \
\n                                                                            \
  imageStore (img_output, p, texel);\n                                        \
}\n";

int main () {
	assert (start_gl ()); // just starts a 4.3 GL context+window
	
	// set up shaders and geometry for full-screen quad
	// moved code to gl_utils.cpp
	GLuint quad_vao = create_quad_vao ();
	GLuint quad_program = create_quad_program ();
	
	GLuint ray_program = 0;
	{ // create the compute shader
		GLuint ray_shader = glCreateShader (GL_COMPUTE_SHADER);
		glShaderSource (ray_shader, 1, &compute_shader_str, NULL);
		glCompileShader (ray_shader);
		assert (check_shader_errors (ray_shader)); // code moved to gl_utils.cpp
		ray_program = glCreateProgram ();
		glAttachShader (ray_program, ray_shader);
		glLinkProgram (ray_program);
		assert (check_program_errors (ray_program)); // code moved to gl_utils.cpp
	}
	
	// texture handle and dimensions
	GLuint tex_output = 0;
	int tex_w = 512, tex_h = 512;
	{ // create the texture
		glGenTextures (1, &tex_output);
		glActiveTexture (GL_TEXTURE0);
		glBindTexture (GL_TEXTURE_2D, tex_output);
		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		// linear allows us to scale the window up retaining reasonable quality
		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		// same internal format as compute shader input
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, tex_w, tex_h, 0, GL_RGBA,
			GL_FLOAT, NULL);
		// bind to image unit so can write to specific pixels from the shader
		glBindImageTexture (0, tex_output, 0, GL_FALSE, 0, GL_WRITE_ONLY,
			GL_RGBA32F);
	}
	
	{ // query up the workgroups
		int work_grp_size[3], work_grp_inv;
		glGetIntegeri_v (GL_MAX_COMPUTE_WORK_GROUP_SIZE, 0, &work_grp_size[0]);
		glGetIntegeri_v (GL_MAX_COMPUTE_WORK_GROUP_SIZE, 1, &work_grp_size[1]);
		glGetIntegeri_v (GL_MAX_COMPUTE_WORK_GROUP_SIZE, 2, &work_grp_size[2]);
		printf ("max local work group sizes x:%i y:%i z:%i\n",
			work_grp_size[0], work_grp_size[1], work_grp_size[2]);
		// TODO what is this again?
		glGetIntegerv (GL_MAX_COMPUTE_WORK_GROUP_INVOCATIONS, &work_grp_inv);
		printf ("max local work group invocations (volume) %i\n", work_grp_inv);
		// TODO quit if workgroup size < chosen texture dims
	}
	
	while (!glfwWindowShouldClose (window)) { // drawing loop
		// do the ray thing every so often
		{ // TODO put timers in
			glUseProgram (ray_program);
			glDispatchCompute (tex_w, tex_h, 1);
		}
		glClear (GL_COLOR_BUFFER_BIT);
		glUseProgram (quad_program);
		glBindVertexArray (quad_vao);
		glActiveTexture (GL_TEXTURE0);
		glBindTexture (GL_TEXTURE_2D, tex_output);
		glDrawArrays (GL_TRIANGLE_STRIP, 0, 4);
		
		glfwPollEvents ();
		if (GLFW_PRESS == glfwGetKey (window, GLFW_KEY_ESCAPE)) {
			glfwSetWindowShouldClose (window, 1);
		}
		glfwSwapBuffers (window);
	}
	
	stop_gl (); // stop glfw, close window
	return 0;
}
