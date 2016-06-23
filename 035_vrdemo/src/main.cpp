#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <stdio.h>
#define VP_WIDTH 1136
#define VP_HEIGHT 640

GLFWwindow* g_win;
GLuint g_vao_tri;
GLuint g_shadprog;

void init_gl(){
	{
		glfwInit();
		glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
		glfwWindowHint(GLFW_SRGB_CAPABLE, GLFW_TRUE);
		glfwWindowHint (GLFW_SAMPLES, 16);
		{ // NOTE OPENGL ES 3 INVOKED HERE
			glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
			glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
			glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
		}
		g_win = glfwCreateWindow(VP_WIDTH, VP_HEIGHT, "VR Demo", NULL, NULL);
		glfwMakeContextCurrent(g_win);
		glewExperimental = GL_TRUE;
		glewInit ();
	}
	{
		const GLubyte* renderer = glGetString (GL_RENDERER); // get renderer string
		const GLubyte* version = glGetString (GL_VERSION); // version as a string
		printf ("Renderer: %s\n", renderer);
		printf ("OpenGL version supported %s\n", version);
	}
	{
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LESS);
		glClearColor(0.3f, 0.3f, 0.3f, 1.0f);
	}
}

void free_gl(){ glfwTerminate(); }

void init_geom() {
	float points[] = {
		 0.0f,  0.5f,  0.0f,
		 0.5f, -0.5f,  0.0f,
		-0.5f, -0.5f,  0.0f
	};
	GLuint vbo = 0;
	glGenBuffers (1, &vbo);
	glBindBuffer (GL_ARRAY_BUFFER, vbo);
	glBufferData (GL_ARRAY_BUFFER, 9 * sizeof (float), points, GL_STATIC_DRAW);
	glGenVertexArrays (1, &g_vao_tri);
	glBindVertexArray (g_vao_tri);
	glEnableVertexAttribArray (0);
	glBindBuffer (GL_ARRAY_BUFFER, vbo);
	glVertexAttribPointer (0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
}

void compile_shader(GLuint shdr_idx){
	glCompileShader (shdr_idx);
	int params = -1;
	glGetShaderiv (shdr_idx, GL_COMPILE_STATUS, &params);
	if (GL_TRUE != params) {
		fprintf (stderr, "ERROR: GL shader index %i did not compile\n", shdr_idx);
		int max_length = 2048;
		int actual_length = 0;
		char log[2048];
		glGetShaderInfoLog (shdr_idx, max_length, &actual_length, log);
		printf ("shader info log for GL index %u:\n%s\n", shdr_idx, log);
	}
}

void link_program(GLuint prog_idx){
	glLinkProgram (prog_idx);
	int params = -1;
	glGetProgramiv (prog_idx, GL_LINK_STATUS, &params);
	if (GL_TRUE != params) {
		fprintf (stderr, "ERROR: could not link shader programme GL index %u\n", prog_idx);
		int max_length = 2048;
		int actual_length = 0;
		char log[2048];
		glGetProgramInfoLog (prog_idx, max_length, &actual_length, log);
		printf ("program info log for GL index %u:\n%s", prog_idx, log);
	}
}

void init_shaders() {
	const char* vertex_shader =
		"#version 300 es\n"
		"in vec3 vp;"
		"void main () {"
		"  gl_Position = vec4 (vp, 1.0);"
		"}";
	const char* fragment_shader =
		"#version 300 es\n"
		"precision mediump float;" // need this
		"out vec4 frag_colour;"
		"void main () {"
		"  frag_colour = vec4 (0.5, 0.0, 0.5, 1.0);"
		"}";
	GLuint vs = glCreateShader (GL_VERTEX_SHADER);
	glShaderSource (vs, 1, &vertex_shader, NULL);
	compile_shader(vs);
	GLuint fs = glCreateShader (GL_FRAGMENT_SHADER);
	glShaderSource (fs, 1, &fragment_shader, NULL);
	compile_shader(fs);
	g_shadprog = glCreateProgram ();
	glAttachShader (g_shadprog, fs);
	glAttachShader (g_shadprog, vs);
	link_program (g_shadprog);
}

void draw_scene(){
	glUseProgram (g_shadprog);
  glBindVertexArray (g_vao_tri);
  glDrawArrays (GL_TRIANGLES, 0, 3);
}

int main(){
	init_gl();
	init_geom();
	init_shaders();
	while(!glfwWindowShouldClose(g_win)) {
		{
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			glViewport(0, 0, VP_WIDTH / 2, VP_HEIGHT);
			draw_scene();
			glViewport(VP_WIDTH / 2, 0, VP_WIDTH / 2, VP_HEIGHT);
			draw_scene();
			glfwSwapBuffers(g_win);
		}
		{
			glfwPollEvents();
			if (GLFW_PRESS == glfwGetKey (g_win, GLFW_KEY_ESCAPE)) {
				glfwSetWindowShouldClose (g_win, 1);
			}
		}
	}
	free_gl();
	return 0;
}
