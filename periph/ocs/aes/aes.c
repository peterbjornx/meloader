//
// Created by pbx on 15/04/19.
//
#include "ocs/aes.h"
#include "user/meloader.h"
#include <stdio.h>
#include <string.h>
#include "log.h"

#define AES_SIZE (0x1000)
extern int trig;
int aes_read( ocs_aes *a, int addr, void *buffer, int count ) {
    int i = 0;
    uint32_t *buf = buffer;
    if ( addr < 0 || addr >= AES_SIZE )
        return 0;
    if ( addr == 0x05c)
        *buf = 2;
    else if ( addr == 0x040)
        *buf = trig==2  ;
    if ( !gpdma_read( &a->aes_gpdma, addr, buffer, count) )
        log(LOG_TRACE, a->name, "read  unknown 0x%03x count:%i val: 0x%08x", addr, count, *buf);
    return 1;
}

int aes_write( ocs_aes *a, int addr, const void *buffer, int count ) {
    const uint32_t *buf = buffer;
    int i;
    if ( addr < 0 || addr >= AES_SIZE )
        return 0;
    if ( !gpdma_write( &a->aes_gpdma, addr, buffer, count) )
        log(LOG_TRACE, a->name, "write unknown 0x%03x count:%i val: 0x%08x", addr, count, *buf);
    return 1;
}
void aes_dma_write( ocs_aes *h, const void *buffer, size_t count ) {
    log(LOG_ERROR, h->name, "bad dma write to aes!!!");
}

void aes_dma_read( ocs_aes *h, void *data, size_t size ) {
    log(LOG_ERROR, h->name, "bad dma read from aes!!!");
}

void aes_load_key( ocs_aes *a, void *data, size_t count ) {
}

int aes_get_result( ocs_aes *a, void *data, size_t count ) {
    return 0;
}

void aes_init( device_instance *parent, ocs_aes *h, char c ) {
    char name[160];
    memset( h, 0, sizeof(ocs_aes) );
    snprintf( name, 160, "%s_aes%c", parent->name, c );
    gpdma_init( &h->aes_gpdma );
    h->name = strdup(name);
    h->aes_gpdma.impl = h;
    h->aes_gpdma.int_read = aes_dma_read;
    h->aes_gpdma.int_write = aes_dma_write;
}