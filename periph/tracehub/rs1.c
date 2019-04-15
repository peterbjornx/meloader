//
// Created by pbx on 15/04/19.
//
#include "meloader.h"
#include "tracehub.h"

int tracehub_mtb_read(int addr, void *buffer, int count ) {
    addr -= TRACEHUB_MTB_BASE;
    if ( addr < 0 || addr >= TRACEHUB_MTB_SIZE )
        return 0;
    if ( addr < TRACEHUB_TSCU_OFFSET )
        tracehub_gth_read(addr - TRACEHUB_GTH_OFFSET, buffer, count);
    return 1;
}

int tracehub_mtb_write( int addr, const void *buffer, int count ) {
    addr -= TRACEHUB_MTB_BASE;
    if ( addr < 0 || addr >= TRACEHUB_MTB_SIZE )
        return 0;
    if ( addr < TRACEHUB_TSCU_OFFSET )
        tracehub_gth_write(addr - TRACEHUB_GTH_OFFSET, buffer, count);
    return 1;
}


mmio_periph tracehub_ftmr = {
        .write = tracehub_ftmr_write,
        .read = tracehub_ftmr_read
};

mmio_periph tracehub_mtb = {
        .write = tracehub_mtb_write,
        .read = tracehub_mtb_read
};

void tracehub_rs1_install() {
    krnl_periph_reg( &tracehub_ftmr );
    krnl_periph_reg( &tracehub_mtb );
}