// WAD Rend - Copyright 2017 Anton Gerdelan <antonofnote@gmail.com>
// C99
#include "wad.h"
#include "apg_data_structs.h"
#include "gl_utils.h"
#include "linmath.h"
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
  int16_t sector_tag;    // which height to use if 2-sided?? TODO
  int16_t right_sidedef; // or -1
  int16_t left_sidedef;  // or -1
} linedef_t;

typedef struct sidedef_t {
  int16_t x_offset;
  int16_t y_offset;
  char upper_texture_name[9];
  char lower_textere_name[9];
  char middle_texture_name[9];
  int16_t sector;
} sidedef_t;

// storted in VERTEXES lump as sequence of raw xy pairs (16-bit ints)
typedef struct vertex_t { int16_t x, y; } vertex_t;

typedef struct sector_t {
  int16_t floor_height;
  int16_t ceil_height;
  char floor_texture_name[9]; // actually 8 + \0
  char ceil_texture_name[9];  // actually 8 + \0
  int16_t light_level;
  int16_t type;
  int16_t tag;                // corresp. to linedef tag
  int linedefs[128];          // i added this
  int nlinedefs;              // and this
  GLuint floor_vao, ceil_vao; // and this
  GLuint floor_vbo, ceil_vbo; // and this
  int nverts;                 // and this
} sector_t;

static char wad_type[5];
static int ndir_entries;
static int dir_addr;
static dir_entry_t *dir_entries;
static int our_map_dir_idx = -1;
static int nlinedefs;
static linedef_t *linedefs;
static int nvertices;
static int nsidedefs;
static sidedef_t *sidedefs;
static vertex_t *vertices;
static int nsectors;
static sector_t *sectors;

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
  {   // extract map stuff
    { // THINGS
      ;
    }
    { // SECTORS -- do this first so can be populated by sidedefs later
      int sectors_lump_idx = our_map_dir_idx + 8;
      nsectors = dir_entries[sectors_lump_idx].sz / 26;
      printf( "%i sectors\n", nsectors );
      sectors = (sector_t *)calloc( sizeof( sector_t ), nsectors );
      fseek( f, dir_entries[sectors_lump_idx].addr, SEEK_SET );
      for ( int sidx = 0; sidx < nsectors; sidx++ ) {
        fread( &sectors[sidx].floor_height, 2, 1, f );
        fread( &sectors[sidx].ceil_height, 2, 1, f );
        fread( sectors[sidx].floor_texture_name, 1, 8, f );
        fread( sectors[sidx].ceil_texture_name, 1, 8, f );
        fread( &sectors[sidx].light_level, 2, 1, f );
        fread( &sectors[sidx].type, 2, 1, f );
        fread( &sectors[sidx].tag, 2, 1, f );
      }
    }
    { // LINEDEFS
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
      }
    }
    { // SIDEDEFS
      int sidedefs_lump_idx = our_map_dir_idx + 3;
      nsidedefs = dir_entries[sidedefs_lump_idx].sz / 30;
      printf( "%i sidedefs\n", nsidedefs );
      sidedefs = (sidedef_t *)calloc( sizeof( sidedef_t ), nsidedefs );
      fseek( f, dir_entries[sidedefs_lump_idx].addr, SEEK_SET );
      for ( int sdidx = 0; sdidx < nsidedefs; sdidx++ ) {
        fread( &sidedefs[sdidx].x_offset, 2, 1, f );
        fread( &sidedefs[sdidx].y_offset, 2, 1, f );
        fread( sidedefs[sdidx].upper_texture_name, 8, 1, f );
        fread( sidedefs[sdidx].lower_textere_name, 8, 1, f );
        fread( sidedefs[sdidx].middle_texture_name, 8, 1, f );
        fread( &sidedefs[sdidx].sector, 2, 1, f );
      }
    }
    { // VERTEXES
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

  } // endgeomblock
  fclose( f );
  return true;
}

// returns num bytes added
int fill_geom( float *geom_buff ) {
  int comp_idx = 0;
  // for each linedef, fetch x,y and x,y from both its verts
  // then extrude up into 2 triangles
  for ( int ldidx = 0; ldidx < nlinedefs; ldidx++ ) {
    int16_t start_vidx = linedefs[ldidx].start_vertex_idx;
    int16_t end_vidx = linedefs[ldidx].end_vertex_idx;
    int16_t start_x = vertices[start_vidx].x;
    int16_t start_y = -vertices[start_vidx].y;
    int16_t end_x = vertices[end_vidx].x;
    int16_t end_y = -vertices[end_vidx].y;
    int16_t left_sidedef = linedefs[ldidx].right_sidedef;
    int16_t right_sidedef = linedefs[ldidx].left_sidedef;
    int16_t left_sector = -1;
    int16_t right_sector = -1;
    int16_t left_ceil = 0;
    int16_t right_ceil = 0;
    int16_t left_floor = 0;
    int16_t right_floor = 0;
    if ( right_sidedef > -1 ) {
      right_sector = sidedefs[right_sidedef].sector;
      if ( right_sector > -1 ) {
        right_ceil = sectors[right_sector].ceil_height;
        right_floor = sectors[right_sector].floor_height;
        // tell the sector to use our line vertices
        sectors[right_sector].linedefs[sectors[right_sector].nlinedefs++] = ldidx;
      }
    }
    if ( left_sidedef > -1 ) {
      left_sector = sidedefs[left_sidedef].sector;
      if ( left_sector > -1 ) {
        left_ceil = sectors[left_sector].ceil_height;
        left_floor = sectors[left_sector].floor_height;
        // tell the sector to use our line vertices
        sectors[left_sector].linedefs[sectors[left_sector].nlinedefs++] = ldidx;
      }
    }
    // printf( "left floor %i left ceil %i\n", left_floor, left_ceil );
    if ( left_sidedef > -1 ) {
      vec3 a = ( vec3 ){ start_x, left_ceil, start_y };
      vec3 b = ( vec3 ){ start_x, left_floor, start_y };
      vec3 c = ( vec3 ){ end_x, left_floor, end_y };
      vec3 normal = normal_from_triangle( a, b, c );
      // two-sided - gets more complex
      if ( right_sidedef > -1 ) {
        // TODO if middle texture also...
        { ; }
        // a lip shows and we draw upper wall
        if ( right_ceil < left_ceil ) {
          int16_t top = left_ceil;
          int16_t bottom = right_ceil;
          geom_buff[comp_idx++] = (float)start_x;
          geom_buff[comp_idx++] = (float)bottom;
          geom_buff[comp_idx++] = (float)start_y;
          geom_buff[comp_idx++] = normal.x;
          geom_buff[comp_idx++] = normal.y;
          geom_buff[comp_idx++] = normal.z;
          geom_buff[comp_idx++] = (float)end_x;
          geom_buff[comp_idx++] = (float)bottom;
          geom_buff[comp_idx++] = (float)end_y;
          geom_buff[comp_idx++] = normal.x;
          geom_buff[comp_idx++] = normal.y;
          geom_buff[comp_idx++] = normal.z;
          geom_buff[comp_idx++] = (float)end_x;
          geom_buff[comp_idx++] = (float)top;
          geom_buff[comp_idx++] = (float)end_y;
          geom_buff[comp_idx++] = normal.x;
          geom_buff[comp_idx++] = normal.y;
          geom_buff[comp_idx++] = normal.z;
          geom_buff[comp_idx++] = (float)end_x;
          geom_buff[comp_idx++] = (float)top;
          geom_buff[comp_idx++] = (float)end_y;
          geom_buff[comp_idx++] = normal.x;
          geom_buff[comp_idx++] = normal.y;
          geom_buff[comp_idx++] = normal.z;
          geom_buff[comp_idx++] = (float)start_x;
          geom_buff[comp_idx++] = (float)top;
          geom_buff[comp_idx++] = (float)start_y;
          geom_buff[comp_idx++] = normal.x;
          geom_buff[comp_idx++] = normal.y;
          geom_buff[comp_idx++] = normal.z;
          geom_buff[comp_idx++] = (float)start_x;
          geom_buff[comp_idx++] = (float)bottom;
          geom_buff[comp_idx++] = (float)start_y;
          geom_buff[comp_idx++] = normal.x;
          geom_buff[comp_idx++] = normal.y;
          geom_buff[comp_idx++] = normal.z;
        }
        // a lip shows and we draw lower wall
        if ( right_floor > left_floor ) {
          int16_t top = right_floor;
          int16_t bottom = left_floor;
          geom_buff[comp_idx++] = (float)start_x;
          geom_buff[comp_idx++] = (float)bottom;
          geom_buff[comp_idx++] = (float)start_y;
          geom_buff[comp_idx++] = normal.x;
          geom_buff[comp_idx++] = normal.y;
          geom_buff[comp_idx++] = normal.z;
          geom_buff[comp_idx++] = (float)end_x;
          geom_buff[comp_idx++] = (float)bottom;
          geom_buff[comp_idx++] = (float)end_y;
          geom_buff[comp_idx++] = normal.x;
          geom_buff[comp_idx++] = normal.y;
          geom_buff[comp_idx++] = normal.z;
          geom_buff[comp_idx++] = (float)end_x;
          geom_buff[comp_idx++] = (float)top;
          geom_buff[comp_idx++] = (float)end_y;
          geom_buff[comp_idx++] = normal.x;
          geom_buff[comp_idx++] = normal.y;
          geom_buff[comp_idx++] = normal.z;
          geom_buff[comp_idx++] = (float)end_x;
          geom_buff[comp_idx++] = (float)top;
          geom_buff[comp_idx++] = (float)end_y;
          geom_buff[comp_idx++] = normal.x;
          geom_buff[comp_idx++] = normal.y;
          geom_buff[comp_idx++] = normal.z;
          geom_buff[comp_idx++] = (float)start_x;
          geom_buff[comp_idx++] = (float)top;
          geom_buff[comp_idx++] = (float)start_y;
          geom_buff[comp_idx++] = normal.x;
          geom_buff[comp_idx++] = normal.y;
          geom_buff[comp_idx++] = normal.z;
          geom_buff[comp_idx++] = (float)start_x;
          geom_buff[comp_idx++] = (float)bottom;
          geom_buff[comp_idx++] = (float)start_y;
          geom_buff[comp_idx++] = normal.x;
          geom_buff[comp_idx++] = normal.y;
          geom_buff[comp_idx++] = normal.z;
        }
        // regular one-piece wall with void behind
      } else {
        int16_t top = left_ceil;
        int16_t bottom = left_floor;
        geom_buff[comp_idx++] = (float)start_x;
        geom_buff[comp_idx++] = (float)bottom;
        geom_buff[comp_idx++] = (float)start_y;
        geom_buff[comp_idx++] = normal.x;
        geom_buff[comp_idx++] = normal.y;
        geom_buff[comp_idx++] = normal.z;
        geom_buff[comp_idx++] = (float)end_x;
        geom_buff[comp_idx++] = (float)bottom;
        geom_buff[comp_idx++] = (float)end_y;
        geom_buff[comp_idx++] = normal.x;
        geom_buff[comp_idx++] = normal.y;
        geom_buff[comp_idx++] = normal.z;
        geom_buff[comp_idx++] = (float)end_x;
        geom_buff[comp_idx++] = (float)top;
        geom_buff[comp_idx++] = (float)end_y;
        geom_buff[comp_idx++] = normal.x;
        geom_buff[comp_idx++] = normal.y;
        geom_buff[comp_idx++] = normal.z;
        geom_buff[comp_idx++] = (float)end_x;
        geom_buff[comp_idx++] = (float)top;
        geom_buff[comp_idx++] = (float)end_y;
        geom_buff[comp_idx++] = normal.x;
        geom_buff[comp_idx++] = normal.y;
        geom_buff[comp_idx++] = normal.z;
        geom_buff[comp_idx++] = (float)start_x;
        geom_buff[comp_idx++] = (float)top;
        geom_buff[comp_idx++] = (float)start_y;
        geom_buff[comp_idx++] = normal.x;
        geom_buff[comp_idx++] = normal.y;
        geom_buff[comp_idx++] = normal.z;
        geom_buff[comp_idx++] = (float)start_x;
        geom_buff[comp_idx++] = (float)bottom;
        geom_buff[comp_idx++] = (float)start_y;
        geom_buff[comp_idx++] = normal.x;
        geom_buff[comp_idx++] = normal.y;
        geom_buff[comp_idx++] = normal.z;
      }
    }

    if ( right_sidedef > -1 ) {
      vec3 a = ( vec3 ){ end_x, right_ceil, end_y };
      vec3 b = ( vec3 ){ end_x, right_floor, end_y };
      vec3 c = ( vec3 ){ start_x, right_floor, start_y };
      vec3 normal = normal_from_triangle( a, b, c );
      // two-sided with upper and/or lower lip and optional middle
      if ( left_sidedef > -1 ) {
        // TODO if middle texture also...
        { ; }
        if ( left_ceil < right_ceil ) {
          int16_t top = right_ceil;
          int16_t bottom = left_ceil;
          geom_buff[comp_idx++] = (float)start_x;
          geom_buff[comp_idx++] = (float)bottom;
          geom_buff[comp_idx++] = (float)start_y;
          geom_buff[comp_idx++] = normal.x;
          geom_buff[comp_idx++] = normal.y;
          geom_buff[comp_idx++] = normal.z;
          geom_buff[comp_idx++] = (float)start_x;
          geom_buff[comp_idx++] = (float)top;
          geom_buff[comp_idx++] = (float)start_y;
          geom_buff[comp_idx++] = normal.x;
          geom_buff[comp_idx++] = normal.y;
          geom_buff[comp_idx++] = normal.z;
          geom_buff[comp_idx++] = (float)end_x;
          geom_buff[comp_idx++] = (float)top;
          geom_buff[comp_idx++] = (float)end_y;
          geom_buff[comp_idx++] = normal.x;
          geom_buff[comp_idx++] = normal.y;
          geom_buff[comp_idx++] = normal.z;
          geom_buff[comp_idx++] = (float)end_x;
          geom_buff[comp_idx++] = (float)top;
          geom_buff[comp_idx++] = (float)end_y;
          geom_buff[comp_idx++] = normal.x;
          geom_buff[comp_idx++] = normal.y;
          geom_buff[comp_idx++] = normal.z;
          geom_buff[comp_idx++] = (float)end_x;
          geom_buff[comp_idx++] = (float)bottom;
          geom_buff[comp_idx++] = (float)end_y;
          geom_buff[comp_idx++] = normal.x;
          geom_buff[comp_idx++] = normal.y;
          geom_buff[comp_idx++] = normal.z;
          geom_buff[comp_idx++] = (float)start_x;
          geom_buff[comp_idx++] = (float)bottom;
          geom_buff[comp_idx++] = (float)start_y;
          geom_buff[comp_idx++] = normal.x;
          geom_buff[comp_idx++] = normal.y;
          geom_buff[comp_idx++] = normal.z;
        }
        if ( left_floor > right_floor ) {
          int16_t top = left_floor;
          int16_t bottom = right_floor;
          geom_buff[comp_idx++] = (float)start_x;
          geom_buff[comp_idx++] = (float)bottom;
          geom_buff[comp_idx++] = (float)start_y;
          geom_buff[comp_idx++] = normal.x;
          geom_buff[comp_idx++] = normal.y;
          geom_buff[comp_idx++] = normal.z;
          geom_buff[comp_idx++] = (float)start_x;
          geom_buff[comp_idx++] = (float)top;
          geom_buff[comp_idx++] = (float)start_y;
          geom_buff[comp_idx++] = normal.x;
          geom_buff[comp_idx++] = normal.y;
          geom_buff[comp_idx++] = normal.z;
          geom_buff[comp_idx++] = (float)end_x;
          geom_buff[comp_idx++] = (float)top;
          geom_buff[comp_idx++] = (float)end_y;
          geom_buff[comp_idx++] = normal.x;
          geom_buff[comp_idx++] = normal.y;
          geom_buff[comp_idx++] = normal.z;
          geom_buff[comp_idx++] = (float)end_x;
          geom_buff[comp_idx++] = (float)top;
          geom_buff[comp_idx++] = (float)end_y;
          geom_buff[comp_idx++] = normal.x;
          geom_buff[comp_idx++] = normal.y;
          geom_buff[comp_idx++] = normal.z;
          geom_buff[comp_idx++] = (float)end_x;
          geom_buff[comp_idx++] = (float)bottom;
          geom_buff[comp_idx++] = (float)end_y;
          geom_buff[comp_idx++] = normal.x;
          geom_buff[comp_idx++] = normal.y;
          geom_buff[comp_idx++] = normal.z;
          geom_buff[comp_idx++] = (float)start_x;
          geom_buff[comp_idx++] = (float)bottom;
          geom_buff[comp_idx++] = (float)start_y;
          geom_buff[comp_idx++] = normal.x;
          geom_buff[comp_idx++] = normal.y;
          geom_buff[comp_idx++] = normal.z;
        }
        // regular wall with void behind
      } else {
        int16_t top = right_ceil;
        int16_t bottom = right_floor;
        geom_buff[comp_idx++] = (float)start_x;
        geom_buff[comp_idx++] = (float)bottom;
        geom_buff[comp_idx++] = (float)start_y;
        geom_buff[comp_idx++] = normal.x;
        geom_buff[comp_idx++] = normal.y;
        geom_buff[comp_idx++] = normal.z;
        geom_buff[comp_idx++] = (float)start_x;
        geom_buff[comp_idx++] = (float)top;
        geom_buff[comp_idx++] = (float)start_y;
        geom_buff[comp_idx++] = normal.x;
        geom_buff[comp_idx++] = normal.y;
        geom_buff[comp_idx++] = normal.z;
        geom_buff[comp_idx++] = (float)end_x;
        geom_buff[comp_idx++] = (float)top;
        geom_buff[comp_idx++] = (float)end_y;
        geom_buff[comp_idx++] = normal.x;
        geom_buff[comp_idx++] = normal.y;
        geom_buff[comp_idx++] = normal.z;
        geom_buff[comp_idx++] = (float)end_x;
        geom_buff[comp_idx++] = (float)top;
        geom_buff[comp_idx++] = (float)end_y;
        geom_buff[comp_idx++] = normal.x;
        geom_buff[comp_idx++] = normal.y;
        geom_buff[comp_idx++] = normal.z;
        geom_buff[comp_idx++] = (float)end_x;
        geom_buff[comp_idx++] = (float)bottom;
        geom_buff[comp_idx++] = (float)end_y;
        geom_buff[comp_idx++] = normal.x;
        geom_buff[comp_idx++] = normal.y;
        geom_buff[comp_idx++] = normal.z;
        geom_buff[comp_idx++] = (float)start_x;
        geom_buff[comp_idx++] = (float)bottom;
        geom_buff[comp_idx++] = (float)start_y;
        geom_buff[comp_idx++] = normal.x;
        geom_buff[comp_idx++] = normal.y;
        geom_buff[comp_idx++] = normal.z;
      }
    }
  } // endfor linedefs
  printf( "%i bytes vertex buffer\n", comp_idx * 4 );
  return comp_idx * 4;
}

float sign( vec2 p1, vec2 p2, vec2 p3 ) {
  return ( p1.x - p3.x ) * ( p2.y - p3.y ) - ( p2.x - p3.x ) * ( p1.y - p3.y );
}

// this is using a Hessian normal form test -- page 176
bool PointInTriangle( vec2 pt, vec2 v1, vec2 v2, vec2 v3 ) {
  bool b1, b2, b3;

  b1 = sign( pt, v1, v2 ) < 0.0f;
  b2 = sign( pt, v2, v3 ) < 0.0f;
  b3 = sign( pt, v3, v1 ) < 0.0f;

  return ( ( b1 == b2 ) && ( b2 == b3 ) );
}

// point inside triangle - john vince page 174
// clockwise point order please
float area_of_tri( vec2 a, vec2 b, vec2 c ) {
  // A = 1/2 [a.x( b.y - c.y) + b.x(c.y - a.y ) + c.x(a.y - b.y )]
  return 0.5f * ( a.x * ( b.y - c.y ) + b.x * ( c.y - a.y ) + c.x * ( a.y - b.y ) );
}

// clockwise point order please
// NOTE if one area is zero and all others positive this point is on boundary
//      if two areas are zero and other positive -> point is on a vertex
bool is_point_in_tri( vec2 probe, vec2 a, vec2 b, vec2 c ) {
  float aa = area_of_tri( a, b, probe );
  float ab = area_of_tri( b, c, probe );
  float ac = area_of_tri( c, a, probe );
  if ( aa > 0.0f && ab > 0.0f && ac > 0.0f ) {
    return true;
  }
  return false;
}

float flat_buffer[4096];
int flat_points;
int flat_comps;

#define PHASE_B
#define PHASE_C
void earclip( int sector_idx ) {
  flat_comps = 0;
  flat_points = 0;
#ifdef PHASE_A
  { // phase 1 - just draw outlines. start with first edge
    for ( int i = 0; i < sectors[sector_idx].nlinedefs; i++ ) {
      int first_linedef_idx = sectors[sector_idx].linedefs[i];
      int first_vertex_idx = linedefs[first_linedef_idx].start_vertex_idx;
      int second_vertex_idx = linedefs[first_linedef_idx].end_vertex_idx;
      flat_buffer[flat_comps++] = (float)vertices[first_vertex_idx].x;
      flat_buffer[flat_comps++] = (float)sectors[sector_idx].floor_height;
      flat_buffer[flat_comps++] = -(float)vertices[first_vertex_idx].y;
      flat_buffer[flat_comps++] = (float)0;
      flat_buffer[flat_comps++] = (float)1;
      flat_buffer[flat_comps++] = (float)0;
      flat_buffer[flat_comps++] = (float)vertices[second_vertex_idx].x;
      flat_buffer[flat_comps++] = (float)sectors[sector_idx].floor_height;
      flat_buffer[flat_comps++] = -(float)vertices[second_vertex_idx].y;
      flat_buffer[flat_comps++] = (float)0;
      flat_buffer[flat_comps++] = (float)1;
      flat_buffer[flat_comps++] = (float)0;
      flat_points += 2;
    }
    sectors[sector_idx].nverts = sectors[sector_idx].nlinedefs * 2;
  }
#endif
#ifdef PHASE_B
  // build adjacent vertex list like this:
  // START - (END/START) - (END/START) -....
  // NOTE ASSUMPTION TO SIMPLIFY -- only 2 linedefs share a vert per sector
  int vert_adjacency[2048] = { -1 };
  // bool slinedef_added[1024] = { false };
  int nlinedefs_added = 0;
  int num_adjacency = 0;
  { // phase 2 - order the vertices
    int first_linedef_idx = sectors[sector_idx].linedefs[0];
    int first_vertex_idx = linedefs[first_linedef_idx].start_vertex_idx;
    int next_vertex_idx = linedefs[first_linedef_idx].end_vertex_idx;

    // reverse if other-sided
    int right_sidedef_idx = linedefs[first_linedef_idx].right_sidedef;
    if ( right_sidedef_idx < 0 ||
         sidedefs[right_sidedef_idx].sector != sector_idx ) {
      first_vertex_idx = linedefs[first_linedef_idx].end_vertex_idx;
      next_vertex_idx = linedefs[first_linedef_idx].start_vertex_idx;
    }

    int last_vertex_idx = first_vertex_idx;
    vert_adjacency[num_adjacency++] = first_vertex_idx;
    //  slinedef_added[0] = true;
    nlinedefs_added++;
    int previous_ld = 0;
    int countdown = 1000;
    while ( nlinedefs_added < sectors[sector_idx].nlinedefs ) {
      if ( last_vertex_idx == next_vertex_idx ) {
        break; // back to the beginning - ignore other lines ASSUMPTION!
        // but weird stuff was adding in after this point
      }
      countdown--;
      assert( countdown );

      // printf("sector %i looking for %i\n", sector_idx, next_vertex_idx);
      for ( int i = 0; i < sectors[sector_idx].nlinedefs; i++ ) {
        int linedef_idx = sectors[sector_idx].linedefs[i];
        int start_v_i = linedefs[linedef_idx].start_vertex_idx;
        int end_v_i = linedefs[linedef_idx].end_vertex_idx;
        // if (sector_idx == 5) {
        // printf("lookat at %i - options %i and %i\n", i, start_v_i, end_v_i);
        //}
        if ( i == previous_ld ) {
          continue;
        }

        if ( start_v_i == next_vertex_idx ) {
          vert_adjacency[num_adjacency++] = start_v_i;
          next_vertex_idx = end_v_i;
          previous_ld = i;
          nlinedefs_added++;
          break;
        } else if ( end_v_i == next_vertex_idx ) {
          vert_adjacency[num_adjacency++] = end_v_i;
          next_vertex_idx = start_v_i;
          previous_ld = i;
          nlinedefs_added++;
          break;
        }
      }
    }
    /*  { // now add them in order
        for ( int i = 0; i < num_adjacency; i++ ) {
          int first_vertex_idx = vert_adjacency[i];
          int b = ( i + 1 ) % num_adjacency;
          int second_vertex_idx = vert_adjacency[b];
          flat_buffer[flat_comps++] = (float)vertices[first_vertex_idx].x;
          flat_buffer[flat_comps++] = (float)sectors[sector_idx].floor_height;
          flat_buffer[flat_comps++] = -(float)vertices[first_vertex_idx].y;
          flat_buffer[flat_comps++] = (float)0;
          flat_buffer[flat_comps++] = (float)1;
          flat_buffer[flat_comps++] = (float)0;
          flat_buffer[flat_comps++] = (float)vertices[second_vertex_idx].x;
          flat_buffer[flat_comps++] = (float)sectors[sector_idx].floor_height;
          flat_buffer[flat_comps++] = -(float)vertices[second_vertex_idx].y;
          flat_buffer[flat_comps++] = (float)0;
          flat_buffer[flat_comps++] = (float)1;
          flat_buffer[flat_comps++] = (float)0;
          flat_points += 2;
        }
        sectors[sector_idx].nverts = sectors[sector_idx].nlinedefs * 2;
      }*/
  }
#endif
#ifdef PHASE_C // find ears
  {
    flat_points = 0;
    flat_comps = 0;
    int remaining_verts = num_adjacency;
    int curr_ptr = 0, prev_ptr = remaining_verts - 1, next_ptr = 1;
    // not > 3 because we add the last triangle on the last go
    while ( remaining_verts > 2 ) {
      //      --- > PART ONE ASSUME EVERYTHING IS AN EAR
      //      --- > PART TWO base on 180 and proper ptrs
      //      --- > PART THREE check if its inside me too
      bool found_ear = true;

      // --------------------look for ear to snip--------------------------

      vec2 prev_vertex, curr_vertex, next_vertex;
      int first_vertex_idx = vert_adjacency[prev_ptr];
      int second_vertex_idx = vert_adjacency[curr_ptr];
      int third_vertex_idx = vert_adjacency[next_ptr];
      {

        prev_vertex.x = (float)vertices[first_vertex_idx].x;
        prev_vertex.y = (float)-vertices[first_vertex_idx].y;
        curr_vertex.x = (float)vertices[second_vertex_idx].x;
        curr_vertex.y = (float)-vertices[second_vertex_idx].y;
        next_vertex.x = (float)vertices[third_vertex_idx].x;
        next_vertex.y = (float)-vertices[third_vertex_idx].y;
      }

      { // angle <= 180  --- NOTE -- looks okay and never finishes the other way
        // around
        vec2 ptoc = sub_vec2_vec2( curr_vertex, prev_vertex );
        vec2 cton = sub_vec2_vec2( next_vertex, curr_vertex );
        vec2 nptoc = normalise_vec2( ptoc );
        vec2 ncton = normalise_vec2( cton );
        float dp = dot_vec2( nptoc, ncton );
        if ( dp < 0.0f ) {
  //        found_ear = false;
  //        break;
        }
      }

      // no vertex inside us (woohoohoo)
      if ( found_ear ) {
        for ( int i = 0; i < num_adjacency; i++ ) {

          int probe_vertex_idx = vert_adjacency[i];
          vec2 probe_vertex;
          probe_vertex.x = (float)vertices[probe_vertex_idx].x;
          probe_vertex.y = (float)-vertices[probe_vertex_idx].y;
          if ( i == prev_ptr ) {
            continue;
          }
          if ( i == curr_ptr ) {
            continue;
          }
          if ( i == next_ptr ) {
            continue;
          }
          if ( is_point_in_tri( probe_vertex, prev_vertex, curr_vertex,
                                next_vertex ) ) {
 //           found_ear = false;
 //           break;
          }
        }
      }

      // stop drop and roll
      if ( found_ear ) {
        { // copy ear into buffer for triangles
          flat_buffer[flat_comps++] = prev_vertex.x;
          flat_buffer[flat_comps++] = sectors[sector_idx].floor_height;
          flat_buffer[flat_comps++] = prev_vertex.y;
          flat_buffer[flat_comps++] = (float)0;
          flat_buffer[flat_comps++] = (float)1;
          flat_buffer[flat_comps++] = (float)0;
          flat_points++;
          flat_buffer[flat_comps++] = curr_vertex.x;
          flat_buffer[flat_comps++] = sectors[sector_idx].floor_height;
          flat_buffer[flat_comps++] = curr_vertex.y;
          flat_buffer[flat_comps++] = (float)0;
          flat_buffer[flat_comps++] = (float)1;
          flat_buffer[flat_comps++] = (float)0;
          flat_points++;
          flat_buffer[flat_comps++] = next_vertex.x;
          flat_buffer[flat_comps++] = sectors[sector_idx].floor_height;
          flat_buffer[flat_comps++] = next_vertex.y;
          flat_buffer[flat_comps++] = (float)0;
          flat_buffer[flat_comps++] = (float)1;
          flat_buffer[flat_comps++] = (float)0;
          flat_points++;
          /*printf( "made tri from %i %i %i\n", first_vertex_idx, second_vertex_idx,
                  third_vertex_idx );*/
        }
        { // shuffle array down
          //printf( "removind vertex %i\n", vert_adjacency[curr_ptr] );
          for ( int i = curr_ptr; i < remaining_verts - 1; i++ ) {
            //printf( "vertex %i was %i\n", i, vert_adjacency[i] );
            vert_adjacency[i] = vert_adjacency[i + 1];
            //printf( "vertex %i is now %i\n", i, vert_adjacency[i] );
          }
          curr_ptr = ( curr_ptr ) % remaining_verts;
          prev_ptr = ( curr_ptr - 1 );
          if ( prev_ptr < 0 ) {
            prev_ptr = remaining_verts - 1;
          }
          next_ptr = ( curr_ptr + 1 ) % remaining_verts;

          /*printf( "cuur vertex is now %i\n", vert_adjacency[curr_ptr] );
          printf( "n vertex is now %i\n", vert_adjacency[next_ptr] );
          printf( "p vertex is now %i\n", vert_adjacency[prev_ptr] );*/

          remaining_verts--;

          /*    curr_ptr = curr_ptr % remaining_verts;
              prev_ptr = curr_ptr - 1;
              if ( prev_ptr < 0 ) {
                prev_ptr = remaining_verts - 1;
              }
              next_ptr = ( curr_ptr + 1 ) % remaining_verts;*/
        }

        // TODO adjust current/rpev/next ptrs
      } else {
        curr_ptr = ( curr_ptr + 1 ) % remaining_verts;
        prev_ptr = ( curr_ptr - 1 );
        if ( prev_ptr < 0 ) {
          prev_ptr = remaining_verts - 1;
        }
        next_ptr = ( curr_ptr + 1 ) % remaining_verts;

        /*printf( "cuur vertex is now %i\n", vert_adjacency[curr_ptr] );
        printf( "n vertex is now %i\n", vert_adjacency[next_ptr] );
        printf( "p vertex is now %i\n", vert_adjacency[prev_ptr] );*/
      }
    }
    { // add last triangle
      /* int first_vertex_idx = vert_adjacency[0];
       int second_vertex_idx = vert_adjacency[1];
       int third_vertex_idx = vert_adjacency[2];
       flat_buffer[flat_comps++] = (float)vertices[first_vertex_idx].x;
       flat_buffer[flat_comps++] = sectors[sector_idx].floor_height;
       flat_buffer[flat_comps++] = -(float)vertices[first_vertex_idx].y;
       flat_buffer[flat_comps++] = (float)0;
       flat_buffer[flat_comps++] = (float)1;
       flat_buffer[flat_comps++] = (float)0;
       flat_points++;

       flat_buffer[flat_comps++] = (float)vertices[second_vertex_idx].x;
       flat_buffer[flat_comps++] = sectors[sector_idx].floor_height;
       flat_buffer[flat_comps++] = -(float)vertices[second_vertex_idx].y;
       flat_buffer[flat_comps++] = (float)0;
       flat_buffer[flat_comps++] = (float)1;
       flat_buffer[flat_comps++] = (float)0;
       flat_points++;

       flat_buffer[flat_comps++] = (float)vertices[third_vertex_idx].x;
       flat_buffer[flat_comps++] = sectors[sector_idx].floor_height;
       flat_buffer[flat_comps++] = -(float)vertices[third_vertex_idx].y;
       flat_buffer[flat_comps++] = (float)0;
       flat_buffer[flat_comps++] = (float)1;
       flat_buffer[flat_comps++] = (float)0;
       flat_points++;*/

      sectors[sector_idx].nverts = flat_points;
    }
  }
#endif
  {
    glGenVertexArrays( 1, &sectors[sector_idx].floor_vao );
    glBindVertexArray( sectors[sector_idx].floor_vao );
    glGenBuffers( 1, &sectors[sector_idx].floor_vbo );
    glBindBuffer( GL_ARRAY_BUFFER, sectors[sector_idx].floor_vbo );
    glBufferData( GL_ARRAY_BUFFER, 6 * flat_points * sizeof( GLfloat ), flat_buffer,
                  GL_STATIC_DRAW );
    GLintptr vertex_normal_offset = 3 * sizeof( float );
    glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof( float ), NULL );
    glVertexAttribPointer( 1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof( float ),
                           (GLvoid *)vertex_normal_offset );
    glEnableVertexAttribArray( 0 );
    glEnableVertexAttribArray( 1 );
  }
}

// nsectors
void fill_sectors() {
  for ( int sectidx = 0; sectidx < nsectors; sectidx++ ) {
    earclip( sectidx );
  }
}

void draw_sectors( int verts ) {
  glDisable( GL_CULL_FACE );

  for ( int sectidx = 0; sectidx < nsectors; sectidx++ ) {
    int usev = verts;
    if ( usev > sectors[sectidx].nverts ) {
      usev = sectors[sectidx].nverts;
    }

    // temp
    usev = sectors[sectidx].nverts;

    glBindVertexArray( sectors[sectidx].floor_vao );
    glDrawArrays( GL_TRIANGLES, 0, usev );
  }

  glEnable( GL_CULL_FACE );
}