#include "spew.h"
#include <stdio.h>

PFNGLENABLEPROC glEnable = NULL;
PFNGLDISABLEPROC glDisable = NULL;
PFNGLGETSTRINGPROC glGetString = NULL;
PFNGLCLEARPROC glClear = NULL;
PFNGLPOLYGONMODEPROC glPolygonMode = NULL;
PFNGLDRAWARRAYSPROC glDrawArrays = NULL;
PFNGLGETINTEGERVPROC glGetIntegerv = NULL;

PFNGLGENBUFFERSPROC glGenBuffers = NULL;
PFNGLBINDBUFFERPROC glBindBuffer = NULL;
PFNGLBUFFERDATAPROC glBufferData = NULL;

PFNGLGENVERTEXARRAYSPROC glGenVertexArrays = NULL;
PFNGLBINDVERTEXARRAYPROC glBindVertexArray = NULL;
PFNGLENABLEVERTEXATTRIBARRAYPROC glEnableVertexAttribArray = NULL;
PFNGLVERTEXATTRIBPOINTERPROC glVertexAttribPointer = NULL;

PFNGLCREATESHADERPROC glCreateShader = NULL;
PFNGLSHADERSOURCEPROC glShaderSource = NULL;
PFNGLCOMPILESHADERPROC glCompileShader = NULL;

PFNGLCREATEPROGRAMPROC glCreateProgram = NULL;
PFNGLATTACHSHADERPROC glAttachShader = NULL;
PFNGLLINKPROGRAMPROC glLinkProgram = NULL;
PFNGLUSEPROGRAMPROC glUseProgram = NULL;
PFNGLPROGRAMUNIFORM1FPROC glProgramUniform1f = NULL;
PFNGLPROGRAMUNIFORM2FPROC glProgramUniform2f = NULL;
PFNGLPROGRAMUNIFORM3FPROC glProgramUniform3f = NULL;
PFNGLPROGRAMUNIFORM4FPROC glProgramUniform4f = NULL;
PFNGLPROGRAMUNIFORMMATRIX4FVPROC glProgramUniformMatrix4fv = NULL;

PFNGLACTIVETEXTUREPROC glActiveTexture = NULL;
PFNGLBINDTEXTUREPROC glBindTexture = NULL;
PFNGLTEXPARAMETERIPROC glTexParameteri = NULL;

PFNGLGENQUERIESPROC glGenQueries = NULL;

// --------------------- EXTENSIONS --------------------

PFNGLDEBUGMESSAGECALLBACKARBPROC glDebugMessageCallbackARB = NULL;

bool spew_fptrs(){
	// format is like this:
	// glCreateProgram =  (PFNGLCREATEPROGRAMPROC)wglGetProcAddress("glCreateProgram");
	
	glEnable = (PFNGLENABLEPROC)glfwGetProcAddress("glEnable");
	glDisable = (PFNGLDISABLEPROC)glfwGetProcAddress("glDisable");
	glGetString = (PFNGLGETSTRINGPROC)glfwGetProcAddress("glGetString");
	glClear = (PFNGLCLEARPROC)glfwGetProcAddress("glClear");
	glPolygonMode = (PFNGLPOLYGONMODEPROC)glfwGetProcAddress("glPolygonMode");
	glDrawArrays = (PFNGLDRAWARRAYSPROC)glfwGetProcAddress("glDrawArrays");
	glGetIntegerv = (PFNGLGETINTEGERVPROC)glfwGetProcAddress("glGetIntegerv");
	
	glGenBuffers = (PFNGLGENBUFFERSPROC)glfwGetProcAddress("glGenBuffers");
	glBindBuffer = (PFNGLBINDBUFFERPROC)glfwGetProcAddress("glBindBuffer");
	glBufferData = (PFNGLBUFFERDATAPROC)glfwGetProcAddress("glBufferData");
	
	glGenVertexArrays = (PFNGLGENVERTEXARRAYSPROC)glfwGetProcAddress("glGenVertexArrays");
	glBindVertexArray = (PFNGLBINDVERTEXARRAYPROC)glfwGetProcAddress("glBindVertexArray");
	glEnableVertexAttribArray = (PFNGLENABLEVERTEXATTRIBARRAYPROC)glfwGetProcAddress("glEnableVertexAttribArray");
	glVertexAttribPointer = (PFNGLVERTEXATTRIBPOINTERPROC)glfwGetProcAddress("glVertexAttribPointer");
	
	glCreateShader = (PFNGLCREATESHADERPROC)glfwGetProcAddress("glCreateShader");
	glShaderSource = (PFNGLSHADERSOURCEPROC)glfwGetProcAddress("glShaderSource");
	glCompileShader = (PFNGLCOMPILESHADERPROC)glfwGetProcAddress("glCompileShader");

	glCreateProgram = (PFNGLCREATEPROGRAMPROC)glfwGetProcAddress("glCreateProgram");
	glAttachShader = (PFNGLATTACHSHADERPROC)glfwGetProcAddress("glAttachShader");
	glLinkProgram = (PFNGLLINKPROGRAMPROC)glfwGetProcAddress("glLinkProgram");
	glUseProgram = (PFNGLUSEPROGRAMPROC)glfwGetProcAddress("glUseProgram");
	glProgramUniform1f = (PFNGLPROGRAMUNIFORM1FPROC)glfwGetProcAddress("glProgramUniform1f");
	glProgramUniform2f = (PFNGLPROGRAMUNIFORM2FPROC)glfwGetProcAddress("glProgramUniform2f");
	glProgramUniform3f = (PFNGLPROGRAMUNIFORM3FPROC)glfwGetProcAddress("glProgramUniform3f");
	glProgramUniform4f = (PFNGLPROGRAMUNIFORM4FPROC)glfwGetProcAddress("glProgramUniform4f");
	glProgramUniformMatrix4fv = (PFNGLPROGRAMUNIFORMMATRIX4FVPROC)glfwGetProcAddress("glProgramUniformMatrix4fv");
	
	glBindTexture = (PFNGLBINDTEXTUREPROC)glfwGetProcAddress("glBindTexture");
	glTexParameteri = (PFNGLTEXPARAMETERIPROC)glfwGetProcAddress("glTexParameteri");
	
	glGenQueries = (PFNGLGENQUERIESPROC)glfwGetProcAddress("glGenQueries");
	
	// --------------------------- EXTENSIONS ---------------------------------
	// non-core but should be available on a toaster since GL 1.1
	if ( glfwExtensionSupported( "GL_EXT_texture_compression_s3tc" )) {
		printf( "EXT_texture_compression_s3tc = yes\n" );
		glDebugMessageCallbackARB = (PFNGLDEBUGMESSAGECALLBACKARBPROC)glfwGetProcAddress("glDebugMessageCallbackARB");
	} else {
		printf( "EXT_texture_compression_s3tc = no\n" );
	}
	
	return true;
}

