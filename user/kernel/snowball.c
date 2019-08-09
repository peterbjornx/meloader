//
// Created by pbx on 16/04/19.
//
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "printf.h"
#include "cfg_file.h"
#include "log.h"
#include "file.h"

typedef struct __attribute__((packed)) {
    char     name[12];
    uint8_t  unk0;
    uint8_t  flags;
    uint16_t size;
    uint16_t unk1;
} romhandoff_block;

typedef struct __attribute__((packed)) {
    uint32_t size;
    void *   buffer;
    uint32_t size2;
    uint32_t actualsize;
} handoff_read_pars;

uint8_t snowball_buffer[0x1FE000];
int     snowball_wrptr = 0;
int     snowball_rdptr = 0;

void snowball_add(const char *name, int unk0, int flags, int size, int unk1,
                  const void *data) {
    int total_size = size + sizeof(romhandoff_block);
    romhandoff_block *block;
    if ( total_size + snowball_wrptr >= sizeof snowball_buffer )
        return;
    block = (romhandoff_block *) (snowball_buffer + snowball_wrptr);
    strncpy(block->name, name, 12);
    block->unk0 = (uint8_t) unk0;
    block->flags = (uint8_t) (flags | 0x1);
    block->unk1 = (uint16_t) unk1;
    block->size = (uint16_t) total_size;
    memcpy(&block[1], data, (size_t) size);
    snowball_wrptr += total_size;
}

int snowball_read(handoff_read_pars *par) {
    int turnsize = par->size;
    int avail = snowball_wrptr - snowball_rdptr;
    mel_printf("[krnl] sys_snowball_read( %i, %p, %i )\n",
            par->size,par->buffer,par->size2);
    par->actualsize = 0;
    if ( par->size != par->size2 )
        return 0;
    if ( !avail )
        return 0;
    if ( avail < turnsize )
        turnsize = avail;
    par->actualsize = (uint32_t) turnsize;
    memcpy(par->buffer, snowball_buffer + snowball_rdptr, (size_t) turnsize);
    snowball_rdptr += turnsize;
    return 0;
}

void snowball_add_section( const cfg_section *section ) {
    uint16_t flags = 0, unk1 = 0, unk0 = 0, size;
    int s,fd;
    uint32_t value_i;
    const cfg_entry *value;
    const char *path, *value_s;
    void *data;
    cfg_find_int16( section, "flags", &flags );
    cfg_find_int16( section, "unknown_0", &unk0 );
    cfg_find_int16( section, "unknown_1", &unk1 );
    path = cfg_find_string( section, "path" );
    if ( path ) {
        data = read_full_file(path, &size);
        snowball_add( section->name, unk0, flags, size, unk1, data );
        free( data );
        return;
    }
    s = cfg_find_int16( section, "size", &size );
    value = cfg_find_entry( section, "value" );
    if ( !value ) {
        logassert( s >= 0, "snowball", "An empty snowball should have a size");
        data = malloc( size );
        logassert( data != NULL, "snowball", "Could not allocate snowball data");
        memset( data, 0, size );
        snowball_add( section->name, unk0, flags, size, unk1, data );
        return;
    }
    if ( value->type == CONFIG_TYPE_STRING ) {
        if ( s < 0 )
            size = strlen(value->string);
        else
            logassert( size <= (strlen(value->string) + 1), "snowball",
                    "Specified size was larger than input string");
        snowball_add( section->name, unk0, flags, size, unk1, value->string );
    } else {
        logassert( s >= 0, "snowball", "An integer snowball should have a size");
        logassert( size <= 8, "snowball", "An integer snowball should be at most 8 bytes");
        snowball_add( section->name, unk0, flags, size, unk1, &value->int64 );
    }
}

void snowball_init( const cfg_file *file ) {
    const cfg_section *section;
    for ( section = file->first_section; section; section = section->next ) {
        if (strcmp(section->type, "snowball") != 0)
            continue;
        snowball_add_section( section );
    }
}