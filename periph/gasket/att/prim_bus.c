//
// Created by pbx on 10/08/19.
//

#include <stddef.h>
#include <log.h>
#include "pci/device.h"
#include "gasket/att/dev.h"

att_window *att_find_win(att_inst *att, int address) {
    for (int i = 0; i < 128; i++) {
        if (~att->regs.WIN[i].CONTROL & 1u)
            continue;
        if (address < att->regs.WIN[i].INT_BA)
            continue;
        if (address >
            (att->regs.WIN[i].INT_BA +
             att->regs.WIN[i].INT_SIZE))
            continue;
        return &att->regs.WIN[i];
    }
    return NULL;
}
int att_prim_read(att_inst *att, int addr, void *buffer, int count, int sai ) {
    att_window *win;
    uint64_t ext;
    win = att_find_win( att, addr );
    if ( !win )
        return -1;
    ext = (((uint64_t)win->EXT_BA_HI) << 32ULL) & 0xFFFFFFFF00000000ULL;
    ext |= win->EXT_BA_LO;
    ext += (addr - win->INT_BA) & 0xFFFFFFFFu;
    return pci_bus_mem_read( &att->prim_bus, ext, buffer, count, sai, 14 );
}

int att_prim_write( att_inst *att, int addr, const void *buffer, int count, int sai ) {
    att_window *win;
    uint64_t ext;
    win = att_find_win( att, addr );
    if ( !win )
        return -1;
    ext = (((uint64_t)win->EXT_BA_HI) << 32ULL) & 0xFFFFFFFF00000000ULL;
    ext |= win->EXT_BA_LO;
    ext += (addr - win->INT_BA) & 0xFFFFFFFFu;
    return pci_bus_mem_write( &att->prim_bus, ext, buffer, count, sai, 14 );
}