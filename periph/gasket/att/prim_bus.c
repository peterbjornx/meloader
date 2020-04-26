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


int att_ret_read(att_inst *att, int addr, void *buffer, int count, int sai ) {
    att_window *win;
    uint64_t ext;
    win = att_find_win( att, addr );
    if ( win )
        return -1;
    return pci_bus_mem_read( att->func.bus, addr, buffer, count, sai, 14 );
}

int att_ret_write( att_inst *att, int addr, const void *buffer, int count, int sai ) {
    att_window *win;
    uint64_t ext;
    win = att_find_win( att, addr );
    if ( win )
        return -1;
    return pci_bus_mem_write( att->func.bus, addr, buffer, count, sai, 14 );
}


static int att_ret_mem_write( pci_func *func, uint64_t addr, const void *buf, int count, int sai, int lat )
{
    att_inst *i = func->device->impl;

    return att_ret_write( i, addr, buf, count, sai );
}

static int att_ret_mem_read( pci_func *func, uint64_t addr,       void *buf, int count, int sai, int lat )
{
    att_inst *i = func->device->impl;

    return att_ret_read( i, addr, buf, count, sai );
}

void att_prim_ret_init( att_inst *i ) {
    i->ret_func.device    = &i->self;
    i->ret_func.mem_read  = att_ret_mem_read;
    i->ret_func.mem_write = att_ret_mem_write;
    i->ret_func.cfg_read  = NULL;
    i->ret_func.cfg_write = NULL;

    pci_func_register( &i->prim_bus, &i->ret_func );
}