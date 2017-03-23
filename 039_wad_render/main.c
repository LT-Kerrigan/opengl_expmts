// WAD Rend - Copyright 2017 Anton Gerdelan <antonofnote@gmail.com>
// C99
#include "gl_utils.h"
#include "linmath.h"
#include "wad.h"
#include <assert.h>
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
    size_t max_geom_sz = 1024 * 1024;
    float *geom_buff = (float *)calloc( 4, max_geom_sz );
    int bytes = fill_geom( geom_buff );
    fill_sectors();
    nwall_verts = bytes / 24;
    assert( bytes <= max_geom_sz );
    glGenVertexArrays( 1, &walls_vao );
    glBindVertexArray( walls_vao );
    GLuint points_vbo;
    glGenBuffers( 1, &points_vbo );
    glBindBuffer( GL_ARRAY_BUFFER, points_vbo );
    glBufferData( GL_ARRAY_BUFFER, 6 * nwall_verts * sizeof( GLfloat ), geom_buff,
                  GL_STATIC_DRAW );
    GLintptr vertex_normal_offset = 3 * sizeof( float );
    glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof( float ), NULL );
    glVertexAttribPointer( 1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof( float ),
                           (GLvoid *)vertex_normal_offset );
    glEnableVertexAttribArray( 0 );
    glEnableVertexAttribArray( 1 );
    free( geom_buff );
  }
  // NOTE: should this be negative to go up? seems very strange... TODO
  vec3 cam_pos = ( vec3 ){ 1059.12, 44, 3651.84 };
  float cam_heading_speed = 100.0f; // 30 degrees per second
  float cam_heading = 0.0f;         // y-rotation in degrees
  mat4 T = translate_mat4( ( vec3 ){ -cam_pos.x, -cam_pos.y, -cam_pos.z } );
  versor quaternion =
    quat_from_axis_deg( -cam_heading, ( vec3 ){ 0.0f, 1.0f, 0.0f } );
  mat4 R = quat_to_mat4( quaternion );
  mat4 view_mat = mult_mat4_mat4( R, T );
  // keep track of some useful vectors that can be used for keyboard movement
  vec4 fwd = ( vec4 ){ 0.0f, 0.0f, -1.0f, 0.0f };
  vec4 rgt = ( vec4 ){ 1.0f, 0.0f, 0.0f, 0.0f };
  vec4 up = ( vec4 ){ 0.0f, 1.0f, 0.0f, 0.0f };

  { // main loops

    glEnable( GL_DEPTH_TEST ); // enable depth-testing
    glDepthFunc( GL_LESS ); // depth-testing interprets a smaller value as "closer"
    // -- dpends on left/right sidedef w/respect to linedef start/end
    glEnable( GL_CULL_FACE ); // cull face
    glCullFace( GL_BACK );    // cull back face
    glFrontFace( GL_CCW );    // set counter-clock-wise vertex order to mean the
    glClearColor( 0.2, 0.2, 0.2, 1.0 ); // grey background to help spot mistakes

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
      { // camera update
        const float cam_speed = 1000.0f;

        bool cam_moved = false;
        vec3 move = ( vec3 ){ 0.0, 0.0, 0.0 };
        float cam_yaw = 0.0f; // y-rotation in degrees
        float cam_pitch = 0.0f;
        float cam_roll = 0.0;

        if ( GLFW_PRESS == glfwGetKey( g_window, GLFW_KEY_W ) ) {
          move.z -= ( cam_speed * elapsed_seconds );
          cam_moved = true;
        }
        if ( GLFW_PRESS == glfwGetKey( g_window, GLFW_KEY_S ) ) {
          move.z += ( cam_speed * elapsed_seconds );
          cam_moved = true;
        }
        if ( GLFW_PRESS == glfwGetKey( g_window, GLFW_KEY_A ) ) {
          move.x -= ( cam_speed * elapsed_seconds );
          cam_moved = true;
        }
        if ( GLFW_PRESS == glfwGetKey( g_window, GLFW_KEY_D ) ) {
          move.x += ( cam_speed * elapsed_seconds );
          cam_moved = true;
        }
        if ( GLFW_PRESS == glfwGetKey( g_window, GLFW_KEY_Q ) ) {
          move.y -= ( cam_speed * elapsed_seconds );
          cam_moved = true;
        }
        if ( GLFW_PRESS == glfwGetKey( g_window, GLFW_KEY_E ) ) {
          move.y += ( cam_speed * elapsed_seconds );
          cam_moved = true;
        }
        if ( glfwGetKey( g_window, GLFW_KEY_LEFT ) ) {
          cam_yaw += cam_heading_speed * elapsed_seconds;
          cam_moved = true;
          versor q_yaw = quat_from_axis_deg( cam_yaw, v3_v4( up ) );
          quaternion = mult_quat_quat( q_yaw, quaternion );
          mat4 R = quat_to_mat4( quaternion );
          fwd = mult_mat4_vec4( R, ( vec4 ){ 0.0, 0.0, -1.0, 0.0 } );
          rgt = mult_mat4_vec4( R, ( vec4 ){ 1.0, 0.0, 0.0, 0.0 } );
          // up = mult_mat4_vec4( R, ( vec4 ){ 0.0, 1.0, 0.0, 0.0 } );
        }
        if ( glfwGetKey( g_window, GLFW_KEY_RIGHT ) ) {
          cam_yaw -= cam_heading_speed * elapsed_seconds;
          cam_moved = true;
          versor q_yaw = quat_from_axis_deg( cam_yaw, v3_v4( up ) );
          quaternion = mult_quat_quat( q_yaw, quaternion );
          mat4 R = quat_to_mat4( quaternion );
          fwd = mult_mat4_vec4( R, ( vec4 ){ 0.0, 0.0, -1.0, 0.0 } );
          rgt = mult_mat4_vec4( R, ( vec4 ){ 1.0, 0.0, 0.0, 0.0 } );
          // up = mult_mat4_vec4( R, ( vec4 ){ 0.0, 1.0, 0.0, 0.0 } );
        }
        if ( glfwGetKey( g_window, GLFW_KEY_UP ) ) {
          cam_pitch += cam_heading_speed * elapsed_seconds;
          cam_moved = true;
          versor q_pitch = quat_from_axis_deg( cam_pitch, v3_v4( rgt ) );
          quaternion = mult_quat_quat( q_pitch, quaternion );
          mat4 R = quat_to_mat4( quaternion );
          fwd = mult_mat4_vec4( R, ( vec4 ){ 0.0, 0.0, -1.0, 0.0 } );
          rgt = mult_mat4_vec4( R, ( vec4 ){ 1.0, 0.0, 0.0, 0.0 } );
          // up = mult_mat4_vec4( R, ( vec4 ){ 0.0, 1.0, 0.0, 0.0 } );
        }
        if ( glfwGetKey( g_window, GLFW_KEY_DOWN ) ) {
          cam_pitch -= cam_heading_speed * elapsed_seconds;
          cam_moved = true;
          versor q_pitch = quat_from_axis_deg( cam_pitch, v3_v4( rgt ) );
          quaternion = mult_quat_quat( q_pitch, quaternion );
          // recalc axes to suit new orientation
          mat4 R = quat_to_mat4( quaternion );
          fwd = mult_mat4_vec4( R, ( vec4 ){ 0.0, 0.0, -1.0, 0.0 } );
          rgt = mult_mat4_vec4( R, ( vec4 ){ 1.0, 0.0, 0.0, 0.0 } );
          // up = mult_mat4_vec4( R, ( vec4 ){ 0.0, 1.0, 0.0, 0.0 } );
        }
        if ( cam_moved ) {
          mat4 R = quat_to_mat4( quaternion );
          cam_pos = add_vec3_vec3( cam_pos, mult_vec3_f( v3_v4( fwd ), -move.z ) );
          cam_pos = add_vec3_vec3( cam_pos, mult_vec3_f( v3_v4( up ), move.y ) );
          cam_pos = add_vec3_vec3( cam_pos, mult_vec3_f( v3_v4( rgt ), move.x ) );
          print_vec3( cam_pos );
          mat4 T = translate_mat4( cam_pos );

          view_mat = mult_mat4_mat4( inverse_mat4( R ),inverse_mat4( T) );
        }
        float aspect = (float)g_gl_width / (float)g_gl_height;
        mat4 proj_mat = perspective( 67, aspect, 10.0, 10000.0 );

        glUseProgram( program );
        glUniformMatrix4fv( view_mat_location, 1, GL_FALSE, view_mat.m );
        glUniformMatrix4fv( proj_mat_location, 1, GL_FALSE, proj_mat.m );
      }

      glViewport( 0, 0, g_gl_width, g_gl_height );
      glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

      glUseProgram( program );
      glBindVertexArray( walls_vao );
      glDrawArrays( GL_TRIANGLES, 0, nwall_verts );

      draw_sectors();

      // put the stuff we've been drawing onto the display
      glfwSwapBuffers( g_window );
    }
  }
  { // closedown
    ;
  }
  return 0;
}