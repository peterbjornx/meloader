//
// Created by pbx on 09/08/19.
//

#ifndef MELOADER_LOADER_H
#define MELOADER_LOADER_H

#include "cfg_file.h"

void insert_thunk(void *wr, void *target );
void insert_thunk_rec(void *wr, void *target );
void romlib_install_thunks( const cfg_section *section );
void snowball_init( const cfg_file *file );

void populate_ranges_mod( me_mod *mod, size_t context_size );
void map_ranges_mod( me_mod *mod );
void populate_ranges_shlib(me_mod *mod);
void populate_ranges_romlib( me_mod *mod, uintptr_t base );
void map_ranges_lib( me_mod *mod );
me_mod *open_mod( const char *modname, int nomet );

#endif //MELOADER_LOADER_H
