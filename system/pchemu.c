//
// Created by pbx on 23/04/20
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
#include "cfg_file.h"

#define LOG_NAME ("pch_emu")

void sven_load( const char *path );

int main( int argc, char **argv ) {
    cfg_file *cfg;
    const char *str;
    const cfg_section *loader;

    logassert( argc == 2, LOG_NAME, "Usage: %s <config file>", argv[0] );

    cfg = load_config( argv[1] );

    loader = cfg_find_section( cfg, "pchemu" );
    logassert( loader != NULL, LOG_NAME, "Could not find pchemu section in config" );

    initialize_devices( cfg );

    str = cfg_find_string( loader, "svendict" );
    if ( str )
        sven_load( str );

    for ( ;; )
        device_process();

}