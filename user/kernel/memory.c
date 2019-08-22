//
// Created by pbx on 05/04/19.
//
#include <string.h>
#include "fsb.h"
#include "log.h"
#include "user/meloader.h"
#include "printf.h"


static mia_fsb *fsb;
static device_instance *cpu;
void krnl_set_cpu( device_instance *_cpu ) {
    fsb = _cpu->impl;
    cpu = _cpu;
}

void krnl_deref_seg( int seg, int offset, size_t count, int *lad ) {
    if ( (seg & 4) != 4 )
        return;//TODO: Support GDT
    seg >>= 3;
    if ( seg >= current_mod->num_segments)
        goto fault;
    if ( (offset + count) > current_mod->segments[seg].limit )
        goto fault;
    *lad = current_mod->segments[seg].base + offset;
    return;
fault:
    *((volatile int *)NULL) = 0xDEADBEEF;
}

void dma_write( int lad, const void *data, size_t count ) {
    log(LOG_FATAL, "krnl", "Using old DMA framework!");
    exit(EXIT_FAILURE);
}

void dma_read ( int lad, void *data, size_t count ) {
    log(LOG_FATAL, "krnl", "Using old DMA framework!");
    exit(EXIT_FAILURE);
}

void krnl_write_seg( int seg, int offset, const void *data, size_t count ) {
    int lad, v=0;
    krnl_deref_seg( seg, offset, count, &lad );
    log(LOG_TRACE,"krnl","Write %08X count %i", lad, count);
    fsb->sa_mem_write( fsb->sa, lad, data, count );
}

void krnl_read_seg ( int seg, int offset, void *data, size_t count ) {
    int lad,v = 0;
    krnl_deref_seg( seg, offset, count, &lad );
    log(LOG_TRACE,"krnl","read  %08X count %i", lad, count);
    fsb->sa_mem_read( fsb->sa, lad, data, count );
}

