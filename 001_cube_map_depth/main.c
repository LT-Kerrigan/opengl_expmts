//
// cube-map shadows test
// first version: anton gerdelan <gerdela@scss.tcd.ie> 18 august 2015
// trinity college dublin, ireland
//

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <stdio.h>

int main () {
	printf ("starting glfw...\n");
	if (!glfwInit ()) {
		fprintf (stderr, "FATAL ERROR: could not start GLFW3\n");
		return 1;
	}

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

	GLFWwindow* window = glfwCreateWindow (800, 600, "cube depth", NULL, NULL);
	if (!window) {
		fprintf (stderr, "FATAL ERROR: could not open window with GLFW3\n");
		glfwTerminate();
		return 1;
	}
	glfwMakeContextCurrent (window);
	
	// start GLEW extension handler
	glewExperimental = GL_TRUE;
	glewInit ();
	
	// get version info
	const GLubyte* renderer = glGetString (GL_RENDERER); // get renderer string
	const GLubyte* version = glGetString (GL_VERSION); // version as a string
	printf ("Renderer: %s\n", renderer);
	printf ("OpenGL version supported %s\n", version);

	
	{
	
	
	
	
	
	
	}
	

	printf ("shutdown\n");
	glfwTerminate();
	return 0;
}
