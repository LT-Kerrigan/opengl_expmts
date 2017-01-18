/* plan:
create text file containing list of all needed GL commands
OR load from gl headers
generate externs for this header from list
generate declarations in c file
and calls to fetch pointers in c file
*/

#pragma once
#include "GLFW/glfw3.h"
#include <stdbool.h>

// i'm adding function pointer externs /as/ i need those functions
//like this
//extern  PFN + ToUppercase(function_name) + PROC
// ref: https://www.opengl.org/discussion_boards/showthread.php/175661-Using-OpenGL-without-GLEW

bool spew_fptrs();

// TODO find out why this old 1.1 stuff is fine...worrying
//extern PFNGLGETSTRINGPROC glGetString;
//glClear
//glDrawArrays

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

