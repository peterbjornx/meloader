//
// Created by pbx on 20/04/19.
//
#include "gasket/att/regs.h"
#include "gasket/att/dev.h"
#include "user/meloader.h"
#include "log.h"
#include "sideband.h"

void att_printfield18(att_sb_window *att) {
    int endpoint = att->sb_address & 0xFF;
    int unk0 = (att->sb_address >> 8) & 0xFF;
    int unk1 = (att->sb_address >> 16) & 0xFF;
    int bar = (att->sb_address >> 24) & 0x7;
    int uf0 = (att->sb_address >> 27) & 0x1F;
    log(LOG_INFO, "att_sbb",
            "ep:0x%02X unk0:0x%02X unk1:0x%02X bar:%i uf0:0x%02x\n",
               endpoint, unk0, unk1, bar, uf0);
}

int att_sb_regs_read(att_inst *att, int addr, void *buffer, int count ) {
    int i = 0;
    uint32_t *buf = buffer;
    if ( addr & 3 ) {
        log( LOG_ERROR, att->self.name,
             "BAR1 Misaligned read 0x%08x %i\n", addr, count);
        return 1;
    }
    i = addr / 32;
    addr = addr % 32;
    switch (addr) {
        case 0x00:
            *buf = att->regs.SB_WIN[i].window_base;
            break;
        case 0x04:
            *buf = att->regs.SB_WIN[i].window_size;
            break;
        case 0x08:
            *buf = att->regs.SB_WIN[i].window_flags;
            break;
        case 0x0C:
            *buf = att->regs.SB_WIN[i].reg_C;
            break;
        case 0x10:
            *buf = att->regs.SB_WIN[i].reg_10;
            break;
        case 0x14:
            *buf = att->regs.SB_WIN[i].reg_14;
            break;
        case 0x18:
            *buf = att->regs.SB_WIN[i].sb_address;
            break;
        case 0x1C:
            *buf = att->regs.SB_WIN[i].reg_1C;
            break;
        default:
            break;
    }
    return 1;
}

int att_sb_regs_write(att_inst *att, int addr, const void *buffer, int count )  {
    const uint32_t *buf = buffer;
    int i;
    if ( addr & 3 ) {
        log( LOG_ERROR, att->self.name,
             "BAR1 Misaligned write 0x%08x %i\n", addr, count);
        return 1;
    }
    i = addr / 32;
    addr = addr % 32;
    switch (addr) {
        case 0x00:
            att->regs.SB_WIN[i].window_base = *buf;
            break;
        case 0x04:
            att->regs.SB_WIN[i].window_size = *buf;
            break;
        case 0x08:
            att->regs.SB_WIN[i].window_flags = *buf;
            break;
        case 0x0C:
            att->regs.SB_WIN[i].reg_C = *buf;
            break;
        case 0x10:
            att->regs.SB_WIN[i].reg_10 = *buf;
            break;
        case 0x14:
            att->regs.SB_WIN[i].reg_14 = *buf;
            break;
        case 0x18:
            att->regs.SB_WIN[i].sb_address = *buf;
            break;
        case 0x1C:
            att->regs.SB_WIN[i].reg_1C = *buf;
            break;
        default:
            break;
    }
    return 1;
}