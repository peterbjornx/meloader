//
// Created by pbx on 15/04/19.
//
#include "sideband.h"
#include "gasket/att/regs.h"
#include "gasket/att/dev.h"
#include "user/meloader.h"
#include "log.h"

att_sb_window *att_find_sb_win(att_inst *att, int address) {
    for (int i = 0; i < 128; i++) {
        if (~att->regs.SB_WIN[i].window_flags & 1)
            continue;
        if (address < att->regs.SB_WIN[i].window_base)
            continue;
        if (address >
            (att->regs.SB_WIN[i].window_base +
             att->regs.SB_WIN[i].window_size))
            continue;
        return &att->regs.SB_WIN[i];
    }
    return NULL;
}

int att_sb_read( att_inst *att, int addr, void *buffer, int count, int sai ) {
    int endp, bar, base;
    att_sb_window *win;
    win = att_find_sb_win( att, addr );
    if ( win ) {
        endp = win->sb_address & 0xFF;
        bar  = (win->sb_address >> 24) & 0x7;
        base = win->window_base;
        //TODO: Add SAI
        sb_read( endp, bar, addr - base, buffer, count );
        return 1;
    } else
        return -1;
}

int att_sb_write( att_inst *att, int addr, const void *buffer, int count, int sai ) {
    int endp, bar, base;
    att_sb_window *win;
    win = att_find_sb_win( att, addr );
    if ( win ) {
        endp = win->sb_address & 0xFF;
        bar  = (win->sb_address >> 24) & 0x7;
        base = win->window_base;
        //TODO: Add SAI
        sb_write( endp, bar, addr - base, buffer, count );
        return 1;
    } else
        return -1;
}