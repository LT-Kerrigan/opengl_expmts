// WAD Rend - Copyright 2017 Anton Gerdelan <antonofnote@gmail.com>
// C99
#pragma once
#include <stdbool.h>

bool open_wad( const char *filename, const char *map_name );
int num_linedefs();
void fill_geom( float* geom_buff );
