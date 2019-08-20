//
// Created by pbx on 09/08/19.
//

#include <sys/mman.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <user/romlib.h>
#include <log.h>
#include <fsb.h>
#include "user/meloader.h"
#include "user/loader.h"
#include "manifest.h"
#include "cfg_file.h"

static me_mod *romlib;
static me_mod *syslib;
static me_mod *module;

void process_thunks (const cfg_section *section ) {
    romlib_install_thunks(section);
    syslib_install_thunks(section);
}

void load_rom( const cfg_section *section ) {
    const char *file;
    uint32_t base;
    int s;

    file = cfg_find_string( section, "path" );
    logassert( file != NULL, "loader", "Could not find ROM path in config" );

    s = cfg_find_int32( section, "base", &base );
    logassert( s >= 0, "loader", "Could not find ROM virtual base address");

    romlib = open_mod( file, 1 );

    populate_ranges_romlib( romlib, base );
    map_ranges_lib( romlib );
    process_thunks( section );

}

void load_syslib( const cfg_section *section ) {
    const char *file;
    int s;

    file = cfg_find_string( section, "path" );
    logassert( file != NULL, "loader", "Could not find library path in config" );

    syslib = open_mod( file, 0 );

    populate_ranges_shlib( syslib );
    map_ranges_lib( syslib );
    process_thunks( section );

}

void load_module( const cfg_section *section ) {
    const char *file;
    int s;

    file = cfg_find_string( section, "path" );
    logassert( file != NULL, "loader", "Could not find module path in config" );

    module = open_mod( file, 0 );

    populate_ranges_mod( module, syslib->context_size );
    map_ranges_mod( module );
    process_thunks( section );

}

int main( int argc, char **argv ) {
    cfg_file *cfg;
    const char *str;
    const cfg_section *section;
    const cfg_section *loader;
    uint32_t main_args[2];
    device_instance *cpu;
    
    logassert( argc == 2, "loader", "Usage: %s <config file>", argv[0] );
    
    cfg = load_config( argv[1] );

    loader = cfg_find_section( cfg, "loader" );
    logassert( loader != NULL, "loader", "Could not find loader section in config" );

    str = cfg_find_string( loader, "romlib" );
    logassert( str != NULL, "loader", "Could not find ROM section name in config" );

    section = cfg_find_section( cfg, str );
    logassert( section != NULL, "loader", "Could not find ROM section %s in config", str );

    load_rom( section );

    str = cfg_find_string( loader, "syslib" );
    logassert( str != NULL, "loader", "Could not find syslib section name in config" );

    section = cfg_find_section( cfg, str );
    logassert( section != NULL, "loader", "Could not find syslib section %s in config", str );

    load_syslib( section );

    str = cfg_find_string( loader, "module" );
    logassert( str != NULL, "loader", "Could not find module section name in config" );

    section = cfg_find_section( cfg, str );
    logassert( section != NULL, "loader", "Could not find module section %s in config", str );

    load_module( section );

    initialize_devices( cfg );
    
    str = cfg_find_string( loader, "cpu" );
    logassert( str != NULL, "loader", "Could not find CPU name in config" );
    
    cpu = device_find( str );
    logassert( section != NULL, "loader", "Could not find CPU %s", str );
    
    krnl_set_cpu( cpu );
    
    

    krnl_set_current_mod( module );
    
    snowball_init( cfg );

    main_args[0] = 0;
    main_args[1] = module->threads[0].thread_id;
    start_thread(0, main_args, sizeof main_args);
    
}