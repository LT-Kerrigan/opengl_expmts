// WAD Rend - Copyright 2017 Anton Gerdelan <antonofnote@gmail.com>
// C99
#include "wad.h"
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
    printf( "left floor %i left ceil %i\n", left_floor, left_ceil );
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

/*
floors/ceils
 // use Ear Clipping algorithm O(n^2) to tessellate sectors
  // NOTE: separate boxes are valid sectors...
*/
void fill_sectors() {
  for ( int sectidx = 0; sectidx < nsectors; sectidx++ ) {
    int nsld = sectors[sectidx].nlinedefs;
    int nsector_verts = nsld * 2;
    sectors[sectidx].nverts = nsector_verts;
    int nvertcomps = 6;
    float *floor_buffer =
      (float *)calloc( nsector_verts * nvertcomps, sizeof( float ) );
    float *ceil_buffer =
      (float *)calloc( nsector_verts * nvertcomps, sizeof( float ) );
    int buffidx = 0;
    // floor
    for ( int ldidx = 0; ldidx < nsld; ldidx++ ) {
      int16_t actualld_idx = sectors[sectidx].linedefs[ldidx];
      int16_t start = linedefs[actualld_idx].start_vertex_idx;
      int16_t end = linedefs[actualld_idx].end_vertex_idx;

      floor_buffer[buffidx++] = (float)vertices[start].x;
      floor_buffer[buffidx++] = (float)sectors[sectidx].floor_height;
      floor_buffer[buffidx++] = -(float)vertices[start].y;
      floor_buffer[buffidx++] = 0.0f;
      floor_buffer[buffidx++] = 1.0f;
      floor_buffer[buffidx++] = 0.0f;
      floor_buffer[buffidx++] = (float)vertices[end].x;
      floor_buffer[buffidx++] = (float)sectors[sectidx].floor_height;
      floor_buffer[buffidx++] = -(float)vertices[end].y;
      floor_buffer[buffidx++] = 0.0f;
      floor_buffer[buffidx++] = 1.0f;
      floor_buffer[buffidx++] = 0.0f;
    }
    // ceil
    buffidx = 0;
    for ( int ldidx = 0; ldidx < nsld; ldidx++ ) {
      int16_t actualld_idx = sectors[sectidx].linedefs[ldidx];
      int16_t start = linedefs[actualld_idx].start_vertex_idx;
      int16_t end = linedefs[actualld_idx].end_vertex_idx;

      ceil_buffer[buffidx++] = (float)vertices[start].x;
      ceil_buffer[buffidx++] = (float)sectors[sectidx].ceil_height;
      ceil_buffer[buffidx++] = -(float)vertices[start].y;
      ceil_buffer[buffidx++] = 0.0f;
      ceil_buffer[buffidx++] = -1.0f;
      ceil_buffer[buffidx++] = 0.0f;
      ceil_buffer[buffidx++] = (float)vertices[end].x;
      ceil_buffer[buffidx++] = (float)sectors[sectidx].ceil_height;
      ceil_buffer[buffidx++] = -(float)vertices[end].y;
      ceil_buffer[buffidx++] = 0.0f;
      ceil_buffer[buffidx++] = -1.0f;
      ceil_buffer[buffidx++] = 0.0f;
    }
    {
      glGenVertexArrays( 1, &sectors[sectidx].ceil_vao );
      glBindVertexArray( sectors[sectidx].ceil_vao );
      glGenBuffers( 1, &sectors[sectidx].ceil_vbo );
      glBindBuffer( GL_ARRAY_BUFFER, sectors[sectidx].ceil_vbo );
      glBufferData( GL_ARRAY_BUFFER, nsector_verts * nvertcomps * sizeof( GLfloat ),
                    ceil_buffer, GL_STATIC_DRAW );
      GLintptr vertex_normal_offset = 3 * sizeof( float );
      glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof( float ), NULL );
      glVertexAttribPointer( 1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof( float ),
                             (GLvoid *)vertex_normal_offset );
      glEnableVertexAttribArray( 0 );
      glEnableVertexAttribArray( 1 );
    }
    {
      glGenVertexArrays( 1, &sectors[sectidx].floor_vao );
      glBindVertexArray( sectors[sectidx].floor_vao );
      glGenBuffers( 1, &sectors[sectidx].floor_vbo );
      glBindBuffer( GL_ARRAY_BUFFER, sectors[sectidx].floor_vbo );
      glBufferData( GL_ARRAY_BUFFER, nsector_verts * nvertcomps * sizeof( GLfloat ),
                    floor_buffer, GL_STATIC_DRAW );
      GLintptr vertex_normal_offset = 3 * sizeof( float );
      glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof( float ), NULL );
      glVertexAttribPointer( 1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof( float ),
                             (GLvoid *)vertex_normal_offset );
      glEnableVertexAttribArray( 0 );
      glEnableVertexAttribArray( 1 );
    }

    free( floor_buffer );
    free( ceil_buffer );
  }
}

// TODO -- maybe subsectors instead?
void draw_sectors() {
  glDisable( GL_CULL_FACE ); // cull face
  for ( int sectidx = 0; sectidx < nsectors; sectidx++ ) {
    glBindVertexArray( sectors[sectidx].floor_vao );
    glDrawArrays( GL_POLYGON, 0, sectors[sectidx].nverts );
  //  glBindVertexArray( sectors[sectidx].ceil_vao );
   // glDrawArrays( GL_POLYGON, 0, sectors[sectidx].nverts );
  }
  glEnable( GL_CULL_FACE ); // cull face
}