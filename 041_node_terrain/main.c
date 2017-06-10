// terrain subdivision test
// anton gerdelan -- 10 jun 2017
// gcc main.c gl_utils.c ../common/lin64/libGLEW.a -lglfw -lGL -lm

// TODO
// * separate shaders for max/mid/min (possibly build render queues)
// * reverse orientation of every 2nd square's triangles
// * use index buffers -- probably will also fix seams if vbo is global

#include "gl_utils.h"
#include "linmath.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <stdio.h>
#include <assert.h>

#define HMAP_IMG "hm2.png"
// horiz dims of node in world units
#define TERR_WIDTH_SCALE 2.0
// distance between entire range of height values in world units
#define TERR_HEIGHT_SCALE (1.0 / 255.0) * 10.0

typedef unsigned char byte;

typedef struct terrain_node_t {
	GLuint max_lod_vao, mid_lod_vao, min_lod_vao;
	GLuint max_lod_npoints, mid_lod_npoints, min_lod_npoints;
} terrain_node_t;

typedef struct terrain_t {
	// terrain nodes [across][down]
	terrain_node_t nodes[2][2];
	int hmap_cols, hmap_rows;
} terrain_t;

terrain_t terrain;

// note: [ ][ ][ ][ ] is 4x4 pixels with 3 squares (6 tris) in-between
//       [ ][ ][ ][ ] so if a node is just 2 tris max then fromx, tox are:
//                                                        (0,1),(1,2),(2,3)
terrain_node_t create_terrain_node( byte *data, int fromx, int tox, int fromy, int toy ) {
	assert( data );
	assert( fromx != tox );

	printf("creating node from (%i,%i) to (%i,%i)\n",fromx,fromy,tox,toy);

	terrain_node_t tn;
	tn.max_lod_npoints = 6;
	tn.mid_lod_npoints = 6;
	tn.min_lod_npoints = 6;

	int tl_idx = fromy * terrain.hmap_cols + fromx;
	int tr_idx = fromy * terrain.hmap_cols + tox;
	int bl_idx = toy * terrain.hmap_cols + fromx;
	int br_idx = toy * terrain.hmap_cols + tox;

	float horiz_fac = TERR_WIDTH_SCALE / (float)(tox - fromx);
	vec3 tl_pt = (vec3){ fromx * horiz_fac, data[tl_idx] * TERR_HEIGHT_SCALE, fromy * horiz_fac };
	vec3 tr_pt = (vec3){ tox * horiz_fac, data[tr_idx] * TERR_HEIGHT_SCALE, fromy * horiz_fac, };
	vec3 bl_pt = (vec3){ fromx * horiz_fac, data[bl_idx] * TERR_HEIGHT_SCALE, toy * horiz_fac, };
	vec3 br_pt = (vec3){ tox * horiz_fac, data[br_idx] * TERR_HEIGHT_SCALE, toy * horiz_fac };

	vec3 pts[6];
	pts[0] = tl_pt;
	pts[1] = bl_pt;
	pts[2] = br_pt;
	pts[3] = br_pt;
	pts[4] = tr_pt;
	pts[5] = tl_pt;

	GLuint min_lod_vbo = 0;
	glGenBuffers( 1, &min_lod_vbo );
	glBindBuffer( GL_ARRAY_BUFFER, min_lod_vbo );
	glBufferData( GL_ARRAY_BUFFER, 3 * sizeof( GLfloat ) * 6, pts, GL_STATIC_DRAW );

	glGenVertexArrays( 1, &tn.min_lod_vao );
	glBindVertexArray( tn.min_lod_vao );
	glEnableVertexAttribArray( 0 );
	glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, 0, NULL );

	return tn;
}

int main() {
	printf(" hello worlds\n" );

	bool result = start_opengl();
	assert( result );

	{ // load image into geom
		int x, y, n;
		byte *data = stbi_load( HMAP_IMG, &x, &y, &n, 0);
		assert( data );
		assert( n == 1 );
		printf( "loaded img with %ix%i, %i chans\n", x, y, n );	
		terrain.hmap_cols = x;
		terrain.hmap_rows = y;

		// work out subset of heightmap to use
		int halfx = x / 2;
		int halfy = y / 2;
		terrain.nodes[0][0] = create_terrain_node( data,     0, halfx, 0, halfy );
		terrain.nodes[0][1] = create_terrain_node( data, halfx, x - 1, 0, halfy );
		terrain.nodes[1][0] = create_terrain_node( data,     0, halfx, halfy, y - 1 );
		terrain.nodes[1][1] = create_terrain_node( data, halfx, x - 1, halfy, y - 1 );

		free( data );

		printf( "terrain nodes created\n" );
	}

	GLuint shader_prog = 0;
	GLint P_loc = -1,V_loc = 1 ;
	{ // some default shader
	const char *vertex_shader = "#version 430\n"
                              "in vec3 vp;"
															"uniform mat4 P, V;"
															"out float h;"
                              "void main () {"
															"h = vp.y;"
                              " gl_Position = P*V*vec4 (vp, 1.0);"
                              "}";
  /* the fragment shader colours each fragment (pixel-sized area of the
  triangle) */
  const char *fragment_shader = "#version 430\n"
																"in float h;"
                                "out vec4 frag_colour;"
                                "void main () {"
                                " frag_colour = vec4 (h*0.1, h*0.1, h*0.1, 1.0);"
                                "}";

		GLuint vert_shader = glCreateShader( GL_VERTEX_SHADER );
	  glShaderSource( vert_shader, 1, &vertex_shader, NULL );
  	glCompileShader( vert_shader );
  	GLuint frag_shader = glCreateShader( GL_FRAGMENT_SHADER );
  	glShaderSource( frag_shader, 1, &fragment_shader, NULL );
  	glCompileShader( frag_shader );
  	shader_prog = glCreateProgram();
  	glAttachShader( shader_prog, frag_shader );
  	glAttachShader( shader_prog, vert_shader );
  	glLinkProgram( shader_prog );
		P_loc = glGetUniformLocation( shader_prog, "P" );
		assert(P_loc >= 0 );
		V_loc = glGetUniformLocation( shader_prog, "V" );
		assert(V_loc >= 0 );
	}

	mat4 P = perspective(67.0f,1.0f, 0.1, 1000.0f);
	mat4 V = look_at(
		(vec3){2,2,10},
		(vec3){2,2,0},
		(vec3){0,1,0}
	);

	glProgramUniformMatrix4fv(shader_prog, P_loc, 1, GL_FALSE, P.m);
	glProgramUniformMatrix4fv(shader_prog, V_loc, 1, GL_FALSE, V.m);


//	glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );

	glDepthFunc( GL_LESS );
  glEnable( GL_DEPTH_TEST );
 // glEnable( GL_CULL_FACE );

	glClearColor(0.2,0.2,0.2,1.0);
	while ( !glfwWindowShouldClose( g_window ) ) {
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

    glUseProgram( shader_prog );

    glBindVertexArray( terrain.nodes[0][0].min_lod_vao );
    glDrawArrays( GL_TRIANGLES, 0, 6 );
    glBindVertexArray( terrain.nodes[0][1].min_lod_vao );
    glDrawArrays( GL_TRIANGLES, 0, 6 );
    glBindVertexArray( terrain.nodes[1][0].min_lod_vao );
    glDrawArrays( GL_TRIANGLES, 0, 6 );
    glBindVertexArray( terrain.nodes[1][1].min_lod_vao );
    glDrawArrays( GL_TRIANGLES, 0, 6 );

    glfwPollEvents();
    glfwSwapBuffers( g_window );
  }

  glfwTerminate();

	return 0;
}
