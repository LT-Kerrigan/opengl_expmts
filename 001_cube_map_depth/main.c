//
// cube-map shadows test
// first version: anton gerdelan <gerdela@scss.tcd.ie> 18 august 2015
// trinity college dublin, ireland
//

#include "apg_maths.h"
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

int g_gl_width = 800;
int g_gl_height = 600;
int g_shad_res = 512;

int main () {
	// GL startup
	// ----------
	GLFWwindow* window = NULL;
	{
		assert (glfwInit ());
	#ifdef APPLE
		glfwWindowHint (GLFW_CONTEXT_VERSION_MAJOR, 3);
		glfwWindowHint (GLFW_CONTEXT_VERSION_MINOR, 2);
		glfwWindowHint (GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
		glfwWindowHint (GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	#else
		glfwWindowHint (GLFW_CONTEXT_VERSION_MAJOR, 4);
		glfwWindowHint (GLFW_CONTEXT_VERSION_MINOR, 3);
		glfwWindowHint (GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
		glfwWindowHint (GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	#endif
		window = glfwCreateWindow (g_gl_width, g_gl_height, "cube depth", NULL,
			NULL);
		assert (window);
		glfwMakeContextCurrent (window);
		// start GLEW extension handler
		glewExperimental = GL_TRUE;
		glewInit ();
		// get version info
		const GLubyte* renderer = glGetString (GL_RENDERER);
		const GLubyte* version = glGetString (GL_VERSION);
		printf ("Renderer: %s\n", renderer);
		printf ("OpenGL version supported %s\n", version);
	}

	// framebuffer
	// -----------
	GLuint fb, depth_tex;
	fb = depth_tex = 0;
	{
		glGenFramebuffers (1, &fb);
		glBindFramebuffer (GL_FRAMEBUFFER, fb);
		// ----
		glGenTextures (1, &depth_tex);
		glActiveTexture (GL_TEXTURE0);
		glBindTexture (GL_TEXTURE_2D, depth_tex);
		glTexImage2D (GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, g_shad_res, g_shad_res,
			0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, NULL);
		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		// attach depth texture to framebuffer
		glFramebufferTexture2D (GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D,
			depth_tex, 0);
		// ----
		GLenum draw_bufs[] = { GL_NONE };
		glDrawBuffers (1, draw_bufs);
		glReadBuffer (GL_NONE);
		glBindFramebuffer (GL_FRAMEBUFFER, 0);
	}

	// scene
	// -----
	GLuint vao = 0;
	{
		float pts[] = { 0.0f, 1.0f, 0.0f, -1.0f, -1.0f, 0.0f, 1.0f, -1.0f, 0.0f };
		GLuint vbo = 0;
		glGenBuffers (1, &vbo);
		glBindBuffer (GL_ARRAY_BUFFER, vbo);
		glBufferData (GL_ARRAY_BUFFER, 9 * sizeof (float), pts, GL_STATIC_DRAW);
		glGenVertexArrays (1, &vao);
		glBindVertexArray (vao);
		glEnableVertexAttribArray (0);
		glBindBuffer (GL_ARRAY_BUFFER, vbo);
		glVertexAttribPointer (0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	}
	
	// camera
	// ------
	mat4 V, P, shad_V, shad_P;
	vec3 cam_pos = vec3_from_3f (0.0f, 0.0f, 1.0f);
	{
		vec3 targ_pos = vec3_from_3f (0.0f, 0.0f, 0.0f);
		vec3 up = vec3_from_3f (0.0f, 1.0f, 0.0f);
		V = look_at (cam_pos, targ_pos, up);
		float aspect = (float)g_gl_width / (float)g_gl_height;
		P = perspective (67.0f, aspect, 0.1f, 100.0f);
		
		cam_pos = vec3_from_3f (0.0f, 0.0f, 3.0f);
		shad_V = look_at (cam_pos, targ_pos, up);
		aspect = (float)g_shad_res / (float)g_shad_res;
		shad_P = perspective (67.0f, aspect, 0.1f, 100.0f);
	}
	
	// shaders
	// -------
	GLuint depth_sp = 0;
	GLint depth_P_loc, depth_V_loc;
	depth_P_loc = depth_V_loc = -1;
	GLuint after_sp = 0;
	GLint after_P_loc, after_V_loc;
	after_P_loc = after_V_loc = -1;
	{
		GLuint vs, fs;
		vs = fs = 0;
		{
			printf ("depth.vert\n");
			GLuint t = GL_VERTEX_SHADER;
			FILE *fp = fopen ("depth.vert", "r");
			
			assert (fp);
			fseek (fp, 0, SEEK_END);
			long sz = ftell (fp);
			rewind (fp);
			char* s_buff = (char*)malloc (sizeof (char) * sz + 1);
			assert (s_buff);
			size_t rs_sz = fread (s_buff, 1, sz, fp);
			printf ("read %i bytes vs\n", (int)rs_sz);
			s_buff[rs_sz] = '\0';
			assert (rs_sz == sz);
			fclose (fp);
			GLuint s = glCreateShader (t);
			const char** p = (const char**)&s_buff;
			glShaderSource (s, 1, p, NULL);
			free (s_buff);
			glCompileShader (s);
			int params = -1;
			glGetShaderiv (s, GL_COMPILE_STATUS, &params);
			if (GL_TRUE != params) {
				fprintf (stderr, "ERROR: GL shader index %i did not compile\n", s);
				int max_length = 2048;
				int actual_length = 0;
				char log[2048];
				glGetShaderInfoLog (s, max_length, &actual_length, log);
				printf ("shader info log for GL index %u:\n%s\n", s, log);
				return 1; // or exit or something
			}
			
			vs = s;
		}
		{
			printf ("depth.frag\n");
			GLuint t = GL_FRAGMENT_SHADER;
			FILE *fp = fopen ("depth.frag", "r");
			
			assert (fp);
			fseek (fp, 0, SEEK_END);
			long sz = ftell (fp);
			rewind (fp);
			char* s_buff = (char*)malloc (sizeof (char) * sz + 1);
			assert (s_buff);
			size_t rs_sz = fread (s_buff, 1, sz, fp);
			printf ("read %i bytes vs\n", (int)rs_sz);
			s_buff[rs_sz] = '\0';
			assert (rs_sz == sz);
			fclose (fp);
			GLuint s = glCreateShader (t);
			const char** p = (const char**)&s_buff;
			glShaderSource (s, 1, p, NULL);
			free (s_buff);
			glCompileShader (s);
			int params = -1;
			glGetShaderiv (s, GL_COMPILE_STATUS, &params);
			if (GL_TRUE != params) {
				fprintf (stderr, "ERROR: GL shader index %i did not compile\n", s);
				int max_length = 2048;
				int actual_length = 0;
				char log[2048];
				glGetShaderInfoLog (s, max_length, &actual_length, log);
				printf ("shader info log for GL index %u:\n%s\n", s, log);
				return 1; // or exit or something
			}
			
			fs = s;
		}
		{
			printf ("depth_sp\n");
			depth_sp = glCreateProgram ();
			glAttachShader (depth_sp, fs);
			glAttachShader (depth_sp, vs);
			glLinkProgram (depth_sp);
			int params = -1;
			glGetProgramiv (depth_sp, GL_LINK_STATUS, &params);
			if (GL_TRUE != params) {
				fprintf (stderr,
					"ERROR: could not link shader programme GL index %u\n",
					depth_sp);
				int max_length = 2048;
				int actual_length = 0;
				char log[2048];
				glGetProgramInfoLog (depth_sp, max_length, &actual_length, log);
				printf ("program info log for GL index %u:\n%s", depth_sp, log);
				return 1;
			}
			depth_P_loc = glGetUniformLocation (depth_sp, "P");
			depth_V_loc = glGetUniformLocation (depth_sp, "V");
			glUseProgram (depth_sp);
			glUniformMatrix4fv (depth_V_loc, 1, GL_FALSE, shad_V.m);
			glUniformMatrix4fv (depth_P_loc, 1, GL_FALSE, shad_P.m);
		}
		{
			printf ("after.vert\n");
			GLuint t = GL_VERTEX_SHADER;
			FILE *fp = fopen ("after.vert", "r");
			
			assert (fp);
			fseek (fp, 0, SEEK_END);
			long sz = ftell (fp);
			rewind (fp);
			char* s_buff = (char*)malloc (sizeof (char) * sz + 1);
			assert (s_buff);
			size_t rs_sz = fread (s_buff, 1, sz, fp);
			printf ("read %i bytes vs\n", (int)rs_sz);
			s_buff[rs_sz] = '\0';
			assert (rs_sz == sz);
			fclose (fp);
			GLuint s = glCreateShader (t);
			const char** p = (const char**)&s_buff;
			glShaderSource (s, 1, p, NULL);
			free (s_buff);
			glCompileShader (s);
			int params = -1;
			glGetShaderiv (s, GL_COMPILE_STATUS, &params);
			if (GL_TRUE != params) {
				fprintf (stderr, "ERROR: GL shader index %i did not compile\n", s);
				int max_length = 2048;
				int actual_length = 0;
				char log[2048];
				glGetShaderInfoLog (s, max_length, &actual_length, log);
				printf ("shader info log for GL index %u:\n%s\n", s, log);
				return 1; // or exit or something
			}
			
			vs = s;
		}
		{
			printf ("after.frag\n");
			GLuint t = GL_FRAGMENT_SHADER;
			FILE *fp = fopen ("after.frag", "r");
			
			assert (fp);
			fseek (fp, 0, SEEK_END);
			long sz = ftell (fp);
			rewind (fp);
			char* s_buff = (char*)malloc (sizeof (char) * sz + 1);
			assert (s_buff);
			size_t rs_sz = fread (s_buff, 1, sz, fp);
			printf ("read %i bytes vs\n", (int)rs_sz);
			s_buff[rs_sz] = '\0';
			assert (rs_sz == sz);
			fclose (fp);
			GLuint s = glCreateShader (t);
			const char** p = (const char**)&s_buff;
			glShaderSource (s, 1, p, NULL);
			free (s_buff);
			glCompileShader (s);
			int params = -1;
			glGetShaderiv (s, GL_COMPILE_STATUS, &params);
			if (GL_TRUE != params) {
				fprintf (stderr, "ERROR: GL shader index %i did not compile\n", s);
				int max_length = 2048;
				int actual_length = 0;
				char log[2048];
				glGetShaderInfoLog (s, max_length, &actual_length, log);
				printf ("shader info log for GL index %u:\n%s\n", s, log);
				return 1; // or exit or something
			}
			
			fs = s;
		}
		{
			printf ("after_sp\n");
			after_sp = glCreateProgram ();
			glAttachShader (after_sp, fs);
			glAttachShader (after_sp, vs);
			glLinkProgram (after_sp);
			int params = -1;
			glGetProgramiv (after_sp, GL_LINK_STATUS, &params);
			if (GL_TRUE != params) {
				fprintf (stderr,
					"ERROR: could not link shader programme GL index %u\n",
					after_sp);
				int max_length = 2048;
				int actual_length = 0;
				char log[2048];
				glGetProgramInfoLog (after_sp, max_length, &actual_length, log);
				printf ("program info log for GL index %u:\n%s", after_sp, log);
				return 1;
			}
			after_P_loc = glGetUniformLocation (after_sp, "P");
			after_V_loc = glGetUniformLocation (after_sp, "V");
			glUseProgram (after_sp);
			glUniformMatrix4fv (after_V_loc, 1, GL_FALSE, V.m);
			glUniformMatrix4fv (after_P_loc, 1, GL_FALSE, P.m);
		}
	}
	
	// states
	// ------
	{
		glEnable (GL_DEPTH_TEST);
		glDepthFunc (GL_LESS);
		glCullFace (GL_BACK);
		glFrontFace (GL_CCW);
		glClearColor (0.2f, 0.2f, 0.2f, 1.0f);
	}
	
	// main loop
	// ---------
	while (!glfwWindowShouldClose (window)) {
	
		// depth write pass
		// ----------------
		glViewport (0, 0, g_shad_res, g_shad_res);
		glBindFramebuffer (GL_FRAMEBUFFER, fb);
		glClear (GL_DEPTH_BUFFER_BIT);
		glUseProgram (depth_sp);
		// TODO Anton -- caster matrices go in here if different from main cam
		glBindVertexArray (vao);
		glDrawArrays (GL_TRIANGLES, 0, 3);
		
		// normal write pass
		// ----------------
		glBindFramebuffer (GL_FRAMEBUFFER, 0);
		glViewport (0, 0, g_gl_width, g_gl_height);
		glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		
		glActiveTexture (GL_TEXTURE0);
		glBindTexture (GL_TEXTURE_2D, depth_tex);
		
		glUseProgram (after_sp);
		glBindVertexArray (vao);
		glDrawArrays (GL_TRIANGLES, 0, 3);
		
		glfwPollEvents ();
		glfwSwapBuffers (window);
	}
	
	// done
	// ----
	{
		printf ("shutdown\n");
		glfwTerminate();
	}
	return 0;
}
