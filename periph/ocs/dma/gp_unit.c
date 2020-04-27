//
// Created by pbx on 26/05/19.
//
#include "ocs/gp.h"
#include "user/meloader.h"
#include <stdio.h>
#include <string.h>
#include "log.h"

#define GP_SIZE (0x1000)

int gp_read( ocs_gp *a, int addr, void *buffer, int count ) {
    int i = 0;
    uint32_t *buf = buffer;
    if ( addr < 0 || addr >= GP_SIZE )
        return 0;
    if ( !gpdma_read( &a->gp_gpdma, addr, buffer, count) )
        log(LOG_TRACE, a->name, "read  unknown 0x%03x count:%i val: 0x%08x", addr, count, *buf);
    return 1;
}

int gp_write( ocs_gp *a, int addr, const void *buffer, int count ) {
    const uint32_t *buf = buffer;
    int i;
    if ( addr < 0 || addr >= GP_SIZE )
        return 0;
    if ( !gpdma_write( &a->gp_gpdma, addr, buffer, count) )
        log(LOG_TRACE, a->name, "write unknown 0x%03x count:%i val: 0x%08x", addr, count, *buf);
    return 1;
}
void gp_dma_write( ocs_gp *h, const void *buffer, size_t count ) {
    log(LOG_ERROR, h->name, "bad dma write to GP!!!");
}

void gp_dma_read( ocs_gp *h, void *data, size_t size ) {
    log(LOG_ERROR, h->name, "bad dma read from gp!!!");
}

void gp_init( device_instance *parent, ocs_gp *h ) {
    char name[160];
    memset( h, 0, sizeof(ocs_gp) );
    snprintf( name, 160, "%s_gp", parent->name );
    gpdma_init( &h->gp_gpdma );
    h->name = strdup(name);
    h->gp_gpdma.impl = h;
    h->gp_gpdma.int_read = gp_dma_read;
    h->gp_gpdma.int_write = gp_dma_write;
}