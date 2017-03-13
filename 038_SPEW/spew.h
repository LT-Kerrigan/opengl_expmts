/* plan:
create text file containing list of all needed GL commands
OR load from gl headers
generate externs for this header from list
generate declarations in c file
and calls to fetch pointers in c file
*/

#pragma once

#define GLFW_INCLUDE_GLCOREARB // do this everywhere glfw3.h is included
//makes the GLFW header include the appropriate extension header for the OpenGL
// or OpenGL ES header selected above after and in addition to that header.
#define GLFW_INCLUDE_GLEXT
#include "GLFW/glfw3.h"
#include <stdbool.h>

// i'm adding function pointer externs /as/ i need those functions
//like this
//extern  PFN + ToUppercase(function_name) + PROC
// ref: https://www.opengl.org/discussion_boards/showthread.php/175661-Using-OpenGL-without-GLEW

bool spew_fptrs();

// Q. find out why this old 1.1 stuff is fine...worrying
// A. it works if i tell GLFW to include GLCOREARB.h instead of GL.H. otherwise
// i need to comment these out
extern PFNGLENABLEPROC glEnable;
extern PFNGLDISABLEPROC glDisable;
extern PFNGLGETSTRINGPROC glGetString;
extern PFNGLCLEARPROC glClear;
extern PFNGLPOLYGONMODEPROC glPolygonMode;
extern PFNGLDRAWARRAYSPROC glDrawArrays;
extern PFNGLGETINTEGERVPROC glGetIntegerv;

extern PFNGLGENBUFFERSPROC glGenBuffers;
extern PFNGLBINDBUFFERPROC glBindBuffer;
extern PFNGLBUFFERDATAPROC glBufferData;

extern PFNGLGENVERTEXARRAYSPROC glGenVertexArrays;
extern PFNGLBINDVERTEXARRAYPROC glBindVertexArray;
extern PFNGLENABLEVERTEXATTRIBARRAYPROC glEnableVertexAttribArray;
extern PFNGLVERTEXATTRIBPOINTERPROC glVertexAttribPointer;

extern PFNGLCREATESHADERPROC glCreateShader;
extern PFNGLSHADERSOURCEPROC glShaderSource;
extern PFNGLCOMPILESHADERPROC glCompileShader;

extern PFNGLCREATEPROGRAMPROC glCreateProgram;
extern PFNGLATTACHSHADERPROC glAttachShader;
extern PFNGLLINKPROGRAMPROC glLinkProgram;
extern PFNGLUSEPROGRAMPROC glUseProgram;
extern PFNGLPROGRAMUNIFORM1FPROC glProgramUniform1f;
extern PFNGLPROGRAMUNIFORM2FPROC glProgramUniform2f;
extern PFNGLPROGRAMUNIFORM3FPROC glProgramUniform3f;
extern PFNGLPROGRAMUNIFORM4FPROC glProgramUniform4f;
extern PFNGLPROGRAMUNIFORMMATRIX4FVPROC glProgramUniformMatrix4fv;

extern PFNGLACTIVETEXTUREPROC glActiveTexture;
extern PFNGLBINDTEXTUREPROC glBindTexture;
extern PFNGLTEXPARAMETERIPROC glTexParameteri;

extern PFNGLGENQUERIESPROC glGenQueries;

//-----------------------------extensions---------------------------
extern PFNGLDEBUGMESSAGECALLBACKARBPROC glDebugMessageCallbackARB;
