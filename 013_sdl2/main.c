//include <SDL2/SDL_mixer.h>
#include <GL/glew.h>
// SDL's own instructions were wrong about this
#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
typedef unsigned char uchar;
typedef unsigned char ubyte;
typedef int32_t int32;
typedef uint32_t uint32;

SDL_Window* window = NULL;

void atexit_shutdown () {
  //Mix_CloseAudio();

  printf ("stopping SDL2...\n");
  // Close and destroy the window
  SDL_DestroyWindow (window);
  // shuts down all SDL systems
  SDL_Quit ();
}

bool compile_shader (const char* file_name, GLuint* shader, GLenum type) {
  char* str = NULL;

  { // get string from file
    FILE* fp = fopen (file_name, "r");
    if (!fp) {
      fprintf (stderr, "ERROR: opening shader file %s\n", file_name);
      return false;
    }
    fseek (fp, 0, SEEK_END);
    size_t sz = ftell (fp);
    rewind (fp);
    str = (char*)malloc (sz + 1);
    if (!str) {
      fprintf (stderr, "ERROR: out of memory reading shader file %s\n",
        file_name);
      fclose (fp);
      return false;
    }
    size_t cnt = fread (str, 1, sz, fp);
    if (cnt == 0) {
      fprintf (stderr, "ERROR: reading shader file %s\n", file_name);
      fclose (fp);
      return false;
    }
    // append \0 to end of file string
    str[sz] = 0;
    fclose (fp);
  }

  { // create shader in GL
    *shader = glCreateShader (type);
    glShaderSource (*shader, 1, (const char**)&str, NULL);
    glCompileShader (*shader);
    int params = -1;
    glGetShaderiv (*shader, GL_COMPILE_STATUS, &params);
    if (GL_TRUE != params) {
      int actual_length = 0;
      char slog[4096];
      glGetShaderInfoLog (*shader, 4096, &actual_length, slog);
      fprintf (stderr, "ERROR: shader index %u did not compile. log:\n%s",
        *shader, slog);
      return false;
    }
  }

  free ((void*)str);
  return true;
}

bool create_shader_prog_from_files (const char* vs_file_name,
  const char* fs_file_name, GLuint* sp) {
  GLuint vs = 0, fs = 0;
  if (!compile_shader (vs_file_name, &vs, GL_VERTEX_SHADER)) {
    return false;
  }
  if (!compile_shader (fs_file_name, &fs, GL_FRAGMENT_SHADER)) {
    return false;
  }
  *sp = glCreateProgram ();
  glAttachShader (*sp, fs);
  glAttachShader (*sp, vs);
  glLinkProgram (*sp);
  glDeleteShader (fs);
  glDeleteShader (vs);
  int params = -1;
  glGetProgramiv (*sp, GL_LINK_STATUS, &params);
  if (GL_TRUE != params) {
    fprintf (stderr, "program %i GL_LINK_STATUS = GL_FALSE\n", *sp);
    int max_length = 2048;
    int actual_length = 0;
    char tlog[2048];
    glGetProgramInfoLog (*sp, max_length, &actual_length, tlog);
    printf ("program info log for GL index %u:\n%s", *sp, tlog);
    return false;
  }
  glValidateProgram (*sp);
  glGetProgramiv (*sp, GL_VALIDATE_STATUS, &params);
  if (GL_TRUE != params) {
    fprintf (stderr, "program %i GL_VALIDATE_STATUS = GL_FALSE\n",
      *sp);
    return false;
  }
  return true;
}

int main () {
  GLuint sp, vao;

  {
    { // SDL version sanity check
      SDL_version compiled, linked;
      SDL_VERSION (&compiled);
      SDL_GetVersion (&linked);
      printf ("We compiled against SDL version %d.%d.%d ...\n",
        compiled.major, compiled.minor, compiled.patch);
      printf ("But we are linking against SDL version %d.%d.%d.\n",
        linked.major, linked.minor, linked.patch);
    }

    printf ("starting SDL2...\n");

    // event, file, threading are init by default
    SDL_Init (SDL_INIT_VIDEO);

    SDL_GL_SetAttribute (SDL_GL_CONTEXT_PROFILE_MASK,
      SDL_GL_CONTEXT_PROFILE_CORE);
    // apparently not required in SDL for OS X
    //SDL_GL_SetAttribute (SDL_GL_CONTEXT_FLAGS,
    //  SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG);
  	SDL_GL_SetAttribute (SDL_GL_CONTEXT_MAJOR_VERSION, 3);
  	SDL_GL_SetAttribute (SDL_GL_CONTEXT_MINOR_VERSION, 2);
    // TODO (anton): really?
    //SDL_GL_SetAttribute (SDL_GL_DOUBLEBUFFER, 1);
    //SDL_GL_SetAttribute (SDL_GL_DEPTH_SIZE, 24);

    { // system info gather
      printf ("  platform: %s\n", SDL_GetPlatform ());
      printf ("  logical CPU cores: %i\n", SDL_GetCPUCount ());
      printf ("  L1 cache line sz: %ikB\n", SDL_GetCPUCacheLineSize ());
      printf ("  RAM %iMB\n", SDL_GetSystemRAM ());
    }

    { // window and video
      // SDL_WINDOW_FULLSCREEN, SDL_WINDOW_FULLSCREEN_DESKTOP,
      // SDL_WINDOW_BORDERLESS, SDL_WINDOW_RESIZABLE, SDL_WINDOW_MAXIMIZED,
      // SDL_WINDOW_INPUT_GRABBED, SDL_WINDOW_ALLOW_HIGHDPI
      // Note: On Apple's OS X you must set the NSHighResolutionCapable
      // Info.plist property to YES, otherwise you will not receive a High DPI
      // OpenGL canvas.
      window = SDL_CreateWindow ("SDL demo", SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED, 1024, 768, SDL_WINDOW_OPENGL);
      if (!window) {
        printf ("FATAL ERROR: could not create window: %s\n", SDL_GetError ());
        return 1;
      }
      // apparently must do this explicitly in SDL
      SDL_GLContext context = SDL_GL_CreateContext (window);
    	if (!context) {
    	  fprintf (stderr, "Failed to create OpenGL context: %s\n",
          SDL_GetError());
    	  exit(1);
    	}

      atexit (atexit_shutdown);

      printf ("starting GLEW...\n");
      glewExperimental = GL_TRUE;
    	glewInit ();
    	if (!glewIsSupported ("GL_VERSION_3_0")) {
    	  fprintf (stderr, "OpenGL 3.0 not available");
    	  exit (1);
    	}

      printf ("renderer: %s\n", glGetString (GL_RENDERER));
      printf ("GL v: %s\n", glGetString (GL_VERSION));

      // vsync? SDL_GL_SetSwapInterval(1); or vsync flag i saw elsewhere?
    }
  }

  {
    glEnable (GL_DEPTH_TEST);
    glDepthFunc (GL_LESS);
    glClearColor (0.2f, 0.2f, 0.2f, 1.0f);

    glGenVertexArrays (1, &vao);
  	glBindVertexArray (vao);
    float pts[] = {-1.0f, -1.0f, -1.0f, 1.0f, 1.0f, -1.0f, 1.0f, 1.0f};
    GLuint vbo;
    glGenBuffers (1, &vbo);
  	glBindBuffer (GL_ARRAY_BUFFER, vbo);
  	glBufferData (GL_ARRAY_BUFFER, sizeof (float) * 8, pts, GL_STATIC_DRAW);
    glEnableVertexAttribArray (0);
    glVertexAttribPointer (0, 2, GL_FLOAT, GL_FALSE, 0, NULL);

    if (!create_shader_prog_from_files ("test.vert", "test.frag", &sp)) {
      return 1;
    }
  }

  {
    glViewport (0, 0, 1024, 768);
    glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUseProgram (sp);
		glBindVertexArray (vao);
    glDisable (GL_CULL_FACE);
    glDrawArrays (GL_TRIANGLE_STRIP, 0, 4);
    SDL_GL_SwapWindow (window);
    SDL_Delay (3000);  // Pause execution for 3000 milliseconds, for example
  }

  // atexit
  return 0;
}
