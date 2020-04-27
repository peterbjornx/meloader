//
// Created by pbx on 23/09/19.
//
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include "log.h"
#include "misa/emu.h"

void misa_rom_write( misa_inst *sa, uint32_t addr, const void *buffer, size_t size ) {
        log( LOG_ERROR, sa->self.name, "ROM write at %08x", addr );
}

void misa_rom_read ( misa_inst *sa, uint32_t addr,       void *buffer, size_t size ) {
        if ( addr > sa->rom_size || ( addr + size ) > sa->rom_size ) {
                log( LOG_ERROR, sa->self.name, "ROM read beyond bounds at %08x", addr );
                return;
        }
        memcpy( buffer, sa->rom_data + addr, size );
}

static void rom_loadimg( misa_inst *sa, const char *path ) {
    int file = open( path, O_RDONLY );
    logassert( file >= 0, sa->self.name, "Could not open image %s!", path);
    off_t size = lseek(file, 0, SEEK_END);
    logassert( size != ( off_t ) -1, sa->self.name, "Could not find size of image %s!", path );
    off_t l = lseek(file, 0, SEEK_SET);
    logassert( l != ( off_t ) -1 , sa->self.name, "Could not seek image %s!", path );
    logassert( size <= sa->rom_size, sa->self.name, "ROM image %s does not fit in array!", path );
    ssize_t r = read( file, sa->rom_data, size );
    logassert( r == size, sa->self.name, "Could not read image %s!", path);
    close(file);
}

void misa_rom_init( misa_inst *sa, const cfg_section *section ) {
        int status;
        const char *path;

        if ( sa->bunit_user )
                return;

        status = cfg_find_int32(section, "rom_size", &sa->rom_size);
        logassert( status >= 0, section->name, "Missing or invalid ROM size" );

        sa->rom_data = malloc( sa->rom_size );
        logassert( sa->rom_data != NULL, section->name, "Could not allocate ROM" );

        memset( sa->rom_data, 0, sa->rom_size ); //TODO: Support loading ROM content

        path = cfg_find_string(section, "rom_image");
        if ( path )
            rom_loadimg( sa, path );
}