//
// Created by pbx on 05/04/19.
//
#include <string.h>
#include "meloader.h"

int peripheral_count = 0;
mmio_periph *peripheral_list[64];

void krnl_periph_reg( mmio_periph * periph ) {

}

void krnl_deref_seg( int seg, int offset, size_t count, int *lad ) {
    if ( (seg & 4) != 4 )
        return;//TODO: Support GDT
    seg >>= 3;
    if ( seg >= current_mod->num_segments)
        goto fault;
    if ( (offset + count) >= current_mod->segments[seg].limit )
        goto fault;
    *lad = current_mod->segments[seg].base + offset;
    return;
fault:
    *((int *)NULL) = 0xDEADBEEF;
}

void krnl_write_seg( int seg, int offset, void *data, size_t count ) {
    int lad;
    krnl_deref_seg( seg, offset, count, &lad );
    write_str("[mmio] write addr: 0x");
    write_hex(8, lad);
    write_str("\n");
    for ( int i = 0; i < peripheral_count; i++ )
        peripheral_list[i]->write( lad, data, count );
}
void krnl_read_seg ( int seg, int offset, void *data, size_t count ) {
    int lad;
    krnl_deref_seg( seg, offset, count, &lad );
    if ( seg != 0x3b ) {
        write_str("[mmio] read addr: 0x");
        write_hex(8, lad);
        write_str("\n");
    }
    for ( int i = 0; i < peripheral_count; i++ )
        peripheral_list[i]->read( lad, data, count );
}
