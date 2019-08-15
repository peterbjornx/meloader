//
// Created by pbx on 10/08/19.
//

#include "gasket/att/regs.h"
#include "gasket/att/dev.h"
#include "user/meloader.h"
#include "log.h"
#include "sideband.h"

int att_prim_regs_read(att_inst *att, int addr, void *buffer, int count ) {
    int i = 0;
    uint32_t *buf = buffer;
    if ( addr & 3 ) {
        log( LOG_ERROR, att->self.name,
             "BAR0 Misaligned read 0x%08x %i\n", addr, count);
        return 1;
    }
    i = addr / 32;
    addr = addr % 32;
    switch (addr) {
        case 0x00:
            *buf = att->regs.WIN[i].INT_BA;
            break;
        case 0x04:
            *buf = att->regs.WIN[i].INT_SIZE;
            break;
        case 0x08:
            *buf = att->regs.WIN[i].EXT_BA_LO;
            break;
        case 0x0C:
            *buf = att->regs.WIN[i].EXT_BA_HI;
            break;
        case 0x10:
            *buf = att->regs.WIN[i].CONTROL;
            break;
        default:
            return -1;
    }
    return 0;
}

int att_prim_regs_write(att_inst *att, int addr, const void *buffer, int count )  {
    const uint32_t *buf = buffer;
    int i;
    if ( addr & 3 ) {
        log( LOG_ERROR, att->self.name,
             "BAR0 Misaligned write 0x%08x %i\n", addr, count);
        return 1;
    }
    i = addr / 32;
    addr = addr % 32;
    switch (addr) {
        case 0x00:
            att->regs.WIN[i].INT_BA = *buf;
            break;
        case 0x04:
            att->regs.WIN[i].INT_SIZE = *buf;
            break;
        case 0x08:
            att->regs.WIN[i].EXT_BA_LO = *buf;
            break;
        case 0x0C:
            att->regs.WIN[i].EXT_BA_HI = *buf;
            break;
        case 0x10:
            att->regs.WIN[i].CONTROL = *buf;
            break;
        default:
            return -1;
    }
    return 1;
}