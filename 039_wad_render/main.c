// WAD Rend - Copyright 2017 Anton Gerdelan <antonofnote@gmail.com>
// C99
// TODO -- base 3d stuff of quaternion camera demo. nice flying interface
#include "gl_utils.h"
#include "wad.h"
#include "linmath.h"
#include <stdio.h>
#include <stdlib.h>

#define WAD_FILE "DOOM.WAD"
#define MAP_NAME "E1M1"
#define VERTEX_SHADER_FILE "test.vert"
#define FRAGMENT_SHADER_FILE "test.frag"

GLuint walls_vao;
int nwall_verts;
GLuint program;
int view_mat_location = -1, proj_mat_location = -1;

int main() {
  { // startup
    printf( "WAD Rend - Anton Gerdelan\n" );
    open_wad( WAD_FILE, MAP_NAME );
    start_opengl();
    program =
      create_programme_from_files( VERTEX_SHADER_FILE, FRAGMENT_SHADER_FILE );
    view_mat_location = glGetUniformLocation( program, "view" );
    proj_mat_location = glGetUniformLocation( program, "proj" );
  }
  { // extract and construct wall geometry
    nwall_verts = 6 * num_linedefs();
    float *geom_buff = (float *)calloc( 4, nwall_verts * 3 );
    fill_geom( geom_buff );
    glGenVertexArrays( 1, &walls_vao );
    glBindVertexArray( walls_vao );
    GLuint points_vbo;
    glGenBuffers( 1, &points_vbo );
    glBindBuffer( GL_ARRAY_BUFFER, points_vbo );
    glBufferData( GL_ARRAY_BUFFER, 3 * nwall_verts * sizeof( GLfloat ), geom_buff,
                  GL_STATIC_DRAW );
    glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, 0, NULL );
    glEnableVertexAttribArray( 0 );
    free( geom_buff );
  }
  vec3 cam_pos = (vec3){1664,3000,-3000};
  vec3 targ_pos = (vec3){1664,0,-3000};
  vec3 up = (vec3){0,0,-1};
  mat4 view_mat = look_at(cam_pos,targ_pos,up);
  float aspect = (float)g_gl_width / (float)g_gl_height;
  mat4 proj_mat = perspective(67, aspect, 10.0, 10000.0);
  { // main loops
    glUseProgram( program );
    glUniformMatrix4fv( view_mat_location, 1, GL_FALSE, view_mat.m );
    glUniformMatrix4fv( proj_mat_location, 1, GL_FALSE, proj_mat.m );

    glEnable( GL_DEPTH_TEST ); // enable depth-testing
    glDepthFunc( GL_LESS ); // depth-testing interprets a smaller value as "closer"
    // -- dpends on left/right sidedef w/respect to linedef start/end
    // glEnable( GL_CULL_FACE ); // cull face
    // glCullFace( GL_BACK );    // cull back face
    // glFrontFace( GL_CCW ); // set counter-clock-wise vertex order to mean the
    // front
    glClearColor( 0.2, 0.2, 0.2, 1.0 ); // grey background to help spot mistakes
    glViewport( 0, 0, g_gl_width, g_gl_height );

    double previous_seconds = glfwGetTime();
    while ( !glfwWindowShouldClose( g_window ) ) {
      // update timers
      double current_seconds = glfwGetTime();
      double elapsed_seconds = current_seconds - previous_seconds;
      previous_seconds = current_seconds;
      _update_fps_counter( g_window );

      // update other events like input handling
      glfwPollEvents();

      if ( GLFW_PRESS == glfwGetKey( g_window, GLFW_KEY_ESCAPE ) ) {
        glfwSetWindowShouldClose( g_window, 1 );
      }

      // wipe the drawing surface clear
      glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

      glUseProgram( program );
      glBindVertexArray( walls_vao );
      glDrawArrays( GL_TRIANGLES, 0, nwall_verts );

      
      // put the stuff we've been drawing onto the display
      glfwSwapBuffers( g_window );
    }
  }
  { // closedown
    ;
  }
  return 0;
}
