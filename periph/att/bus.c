//
// Created by pbx on 15/04/19.
//
#include "sideband.h"
#include "att.h"
#include "meloader.h"
#include "log.h"

int att_bus_read(int addr, void *buffer, int count ) {
    int endp, bar, base;
    att_sb_window *win;
    win = att_find_sb_win( addr );
    if ( win ) {
        endp = win->sb_address & 0xFF;
        bar  = (win->sb_address >> 24) & 0x7;
        base = win->window_base;
        sb_read( endp, bar, addr - base, buffer, count );
        return 1;
    } else
        return 0;
}

int att_bus_write( int addr, const void *buffer, int count ) {
    int endp, bar, base;
    att_sb_window *win;
    win = att_find_sb_win( addr );
    if ( win ) {
        endp = win->sb_address & 0xFF;
        bar  = (win->sb_address >> 24) & 0x7;
        base = win->window_base;
        sb_write( endp, bar, addr - base, buffer, count );
        return 1;
    } else
        return 0;
}

mmio_periph att_bus_mmio = {
        .write = att_bus_write,
        .read = att_bus_read
};

void att_bus_install() {
    krnl_periph_reg( &att_bus_mmio );
}

