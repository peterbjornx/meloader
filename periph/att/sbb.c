//
// Created by pbx on 20/04/19.
//
#include "att.h"
#include "meloader.h"
#include "log.h"
#include "sideband.h"

#define ATT_SBB_BASE (0xF00A9000);
#define ATT_SBB_SIZE (0x1000)

att_sb_window att_sb_windows[128];

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

int att_sbb_read(int addr, void *buffer, int count) {
    int i = 0;
    uint32_t *buf = buffer;
    addr -= ATT_SBB_BASE;
    if (addr < 0 || addr >= ATT_SBB_SIZE)
        return 0;
    if ( count != 4 || addr & 3 ) {
        log( LOG_ERROR, "att_sbb",
             "Misaligned read 0x%08x %i\n", addr, count);
        return 1;
    }
    i = addr / 32;
    addr = addr % 32;
    switch (addr) {
        case 0x00:
            *buf = att_sb_windows[i].window_base;
            break;
        case 0x04:
            *buf = att_sb_windows[i].window_size;
            break;
        case 0x08:
            *buf = att_sb_windows[i].window_flags;
            break;
        case 0x0C:
            *buf = att_sb_windows[i].reg_C;
            break;
        case 0x10:
            *buf = att_sb_windows[i].reg_10;
            break;
        case 0x14:
            *buf = att_sb_windows[i].reg_14;
            break;
        case 0x18:
            *buf = att_sb_windows[i].sb_address;
            break;
        case 0x1C:
            *buf = att_sb_windows[i].reg_1C;
            break;
        default:
            break;
    }
    return 1;
}

int att_sbb_write(int addr, const void *buffer, int count) {
    const uint32_t *buf = buffer;
    int i;
    addr -= ATT_SBB_BASE;
    if (addr < 0 || addr >= ATT_SBB_SIZE)
        return 0;
    if ( count != 4 || addr & 3 ) {
        log( LOG_ERROR, "att_sbb",
                "Misaligned write 0x%08x %i\n", addr, count);
        return 1;
    }
    i = addr / 32;
    addr = addr % 32;
    switch (addr) {
        case 0x00:
            att_sb_windows[i].window_base = *buf;
            break;
        case 0x04:
            att_sb_windows[i].window_size = *buf;
            break;
        case 0x08:
            att_sb_windows[i].window_flags = *buf;
            break;
        case 0x0C:
            att_sb_windows[i].reg_C = *buf;
            break;
        case 0x10:
            att_sb_windows[i].reg_10 = *buf;
            break;
        case 0x14:
            att_sb_windows[i].reg_14 = *buf;
            break;
        case 0x18:
            att_sb_windows[i].sb_address = *buf;
            break;
        case 0x1C:
            att_sb_windows[i].reg_1C = *buf;
            break;
        default:
            break;
    }
    return 1;
}

mmio_periph att_sbb_mmio = {
        .write = att_sbb_write,
        .read = att_sbb_read
};

void att_sbb_install() {
    krnl_periph_reg( &att_sbb_mmio );
}

att_sb_window *att_find_sb_win(int address) {
    for (int i = 0; i < 128; i++) {
        if (~att_sb_windows[i].window_flags & 1)
            continue;
        if (address < att_sb_windows[i].window_base)
            continue;
        if (address >
            (att_sb_windows[i].window_base +
             att_sb_windows[i].window_size))
            continue;
        return &att_sb_windows[i];
    }
    return NULL;
}