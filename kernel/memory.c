//
// Created by pbx on 05/04/19.
//
#include <string.h>
#include "meloader.h"
#include "printf.h"
#include "att.h"

int peripheral_count = 0;
mmio_periph *peripheral_list[64];

void krnl_periph_reg( mmio_periph * periph ) {
    peripheral_list[peripheral_count++] = periph;
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

int periph_write( int lad, const void *data, size_t count) {
    int v = 0;
    for ( int i = 0; i < peripheral_count; i++ )
        v |= peripheral_list[i]->write( lad, data, count );
    return v;
}

int periph_read ( int lad, void *data, size_t count ){
    int v = 0;
    for ( int i = 0; i < peripheral_count; i++ )
        v |= peripheral_list[i]->read( lad, data, count );
    return v;
}

void dma_write( int lad, const void *data, size_t count ) {
    if (!periph_write(lad, data, count))
        memcpy( lad, data, count );

}

void dma_read ( int lad, void *data, size_t count ) {
    if (!periph_read (lad, data, count))
        memcpy( data, lad, count );

}

void krnl_write_seg( int seg, int offset, void *data, size_t count ) {
    int lad, v=0;
    krnl_deref_seg( seg, offset, count, &lad );
    if (!periph_write (lad, data, count))
        mel_printf("[mmio] Unimplemented write addr: 0x%08x count: %i\n", lad, count);
}

void krnl_read_seg ( int seg, int offset, void *data, size_t count ) {
    int lad,v = 0;
    krnl_deref_seg( seg, offset, count, &lad );
    if ((!periph_read (lad, data, count)) && seg != 0x3b ) {
        mel_printf("[mmio] Unimplemented read addr:  0x%08x count: %i\n", lad, count);
    }
}

