// WAD Rend - Copyright 2017 Anton Gerdelan <antonofnote@gmail.com>
// C99
#include "wad.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct dir_entry_t {
  int addr;
  int sz;
  char name[9];
} dir_entry_t;

typedef struct linedef_t {
  int16_t start_vertex_idx;
  int16_t end_vertex_idx;
  uint16_t flags;
  int16_t special_type;
  int16_t sector_tag;
  int16_t right_sidedef; // or -1
  int16_t left_sidedef;  // or -1
} linedef_t;

// storted in VERTEXES lump as sequence of raw xy pairs (16-bit ints)
typedef struct vertex_t { int16_t x, y; } vertex_t;

static char wad_type[5];
static int ndir_entries;
static int dir_addr;
static dir_entry_t *dir_entries;
static int our_map_dir_idx = -1;
static int nlinedefs;
static linedef_t *linedefs;
static int nvertices;
static vertex_t *vertices;

int num_linedefs() { return nlinedefs; }

// custom strcmp to avoid commonly-made ==0 bracket soup bugs
// returns true if true so far and one string shorter e.g. "ANT" "ANTON"
bool apg_strmatchy( const char *a, const char *b ) {
  int len_a = strlen( a );
  int len_b = strlen( b );
  for ( int i = 0; ( i < len_a && i < len_b ); i++ ) {
    if ( a[i] != b[i] ) {
      return false;
    }
  }
  return true;
}

/* --> going to assume these are always in this order (DOOM registered v ~1993)
E1M1 THINGS LINEDEFS SIDEDEFS VERTEXES SEG
S SSECTORS NODES SECTORS REJECT BLOCKMAP E1M2

hexen has diff format: http://doom.wikia.com/wiki/Linedef

every linedef is
  - between 2 VERTICES
  - contains one or two SIDEDEFS (contain wall textures)
    - one sidedef is one-sided - void on other side
    - two sidedefs between sectors with any diff properties e.g. height, texture...
  - divide map into SECTORS
  - trigger action specials (action depends on linedef type number, object based on
tag e.g. sector number to affect except doors - always sector on other side of line)
*/

bool open_wad( const char *filename, const char *map_name ) {
  FILE *f = fopen( filename, "rb" );
  if ( !f ) {
    fprintf( stderr, "ERROR: could not open file `%s`\n", filename );
    return false;
  }
  { // header
    fread( wad_type, 1, 4, f );
    printf( "%s\n", wad_type );
    fread( &ndir_entries, 4, 1, f );
    printf( "%i directory entries\n", ndir_entries );
    fread( &dir_addr, 4, 1, f );
    printf( "%i directory address\n", dir_addr );
    dir_entries = (dir_entry_t *)calloc( sizeof( dir_entry_t ), ndir_entries );
  }
  { // directory
    fseek( f, dir_addr, SEEK_SET );
    for ( int dir_idx = 0; dir_idx < ndir_entries; dir_idx++ ) {
      fread( &dir_entries[dir_idx].addr, 4, 1, f );
      fread( &dir_entries[dir_idx].sz, 4, 1, f );
      fread( &dir_entries[dir_idx].name, 1, 8, f );
      if ( apg_strmatchy( map_name, dir_entries[dir_idx].name ) ) {
        printf( "map found in directory at index %i\n", dir_idx );
        our_map_dir_idx = dir_idx;
      }
      // printf( "%s ", dir_entries[dir_idx].name );
    }
  }
  { // extract map stuff
    int linedefs_lump_idx = our_map_dir_idx + 2;
    nlinedefs = dir_entries[linedefs_lump_idx].sz / 14;
    printf( "%i linedefs\n", nlinedefs );
    linedefs = (linedef_t *)calloc( sizeof( linedef_t ), nlinedefs );
    fseek( f, dir_entries[linedefs_lump_idx].addr, SEEK_SET );
    for ( int ldidx = 0; ldidx < nlinedefs; ldidx++ ) {
      fread( &linedefs[ldidx].start_vertex_idx, 2, 1, f );
      fread( &linedefs[ldidx].end_vertex_idx, 2, 1, f );
      fread( &linedefs[ldidx].flags, 2, 1, f );
      fread( &linedefs[ldidx].special_type, 2, 1, f );
      fread( &linedefs[ldidx].sector_tag, 2, 1, f );
      fread( &linedefs[ldidx].right_sidedef, 2, 1, f );
      fread( &linedefs[ldidx].left_sidedef, 2, 1, f );
      /*printf("ld startv %i endv %i flags %04x type %i\n",
        linedefs[ldidx].start_vertex_idx,
        linedefs[ldidx].end_vertex_idx,
        linedefs[ldidx].flags,
        linedefs[ldidx].special_type);*/
    }
    int vertexes_lump_idx = our_map_dir_idx + 4;
    nvertices = dir_entries[vertexes_lump_idx].sz / sizeof( vertex_t );
    printf( "%i vertices\n", nvertices );
    vertices = (vertex_t *)calloc( sizeof( vertex_t ), nvertices );
    fseek( f, dir_entries[vertexes_lump_idx].addr, SEEK_SET );
    for ( int vidx = 0; vidx < nvertices; vidx++ ) {
      fread( &vertices[vidx].x, 2, 1, f );
      fread( &vertices[vidx].y, 2, 1, f );
    }
  }
  fclose( f );
  return true;
}

void fill_geom( float *geom_buff ) {
  int comp_idx = 0;
  // for each linedef, fetch x,y and x,y from both its verts
  // then extrude up into 2 triangles
  for ( int ldidx = 0; ldidx < nlinedefs; ldidx++ ) {
    int start_vidx = linedefs[ldidx].start_vertex_idx;
    int end_vidx = linedefs[ldidx].end_vertex_idx;
    int16_t start_x = vertices[start_vidx].x;
    int16_t start_y = vertices[start_vidx].y;
    int16_t end_x = vertices[end_vidx].x;
    int16_t end_y = vertices[end_vidx].y;
    printf("x,y %f,%f to x,y %f,%f\n", (float)start_x, (float)start_y, (float)end_x, (float)end_y);
    geom_buff[comp_idx++] = (float)start_x;
    geom_buff[comp_idx++] = 0.0f;
    geom_buff[comp_idx++] = (float)start_y;
    geom_buff[comp_idx++] = (float)end_x;
    geom_buff[comp_idx++] = 0.0f;
    geom_buff[comp_idx++] = (float)end_y;
    geom_buff[comp_idx++] = (float)end_x;
    geom_buff[comp_idx++] = 256.0f;
    geom_buff[comp_idx++] = (float)end_y;
    geom_buff[comp_idx++] = (float)end_x;
    geom_buff[comp_idx++] = 256.0f;
    geom_buff[comp_idx++] = (float)end_y;
    geom_buff[comp_idx++] = (float)start_x;
    geom_buff[comp_idx++] = 256.0f;
    geom_buff[comp_idx++] = (float)start_y;
    geom_buff[comp_idx++] = (float)start_x;
    geom_buff[comp_idx++] = 0.0f;
    geom_buff[comp_idx++] = (float)start_y;
  }
  printf( "%i bytes vertex buffer\n", comp_idx * 4 );
}