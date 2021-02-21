//
// Created by pbx on 15/04/19.
//
#include <hwif.h>
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
    int base,i;
    att_sb_window *win;
    win = att_find_sb_win( att, addr );
    if ( win ) {
        base = win->window_base;
        i= sb_read(
                    ATT_SBADDR_ENDPT( win->sb_address ),
                    ATT_SBADDR_RDOP( win->sb_address ),
                    ATT_SBADDR_BAR( win->sb_address ),
                    addr - base, buffer, count, sai );
        if ( i == -2 ) {
            hwif_sbaddr adr;
            adr.endpt = ATT_SBADDR_ENDPT(win->sb_address);
            adr.bar = ATT_SBADDR_BAR( win->sb_address );
            adr.rd_op = ATT_SBADDR_RDOP( win->sb_address );
            adr.wr_op = ATT_SBADDR_WROP( win->sb_address );
            adr.rs = ATT_SBADDH_RS( win->reg_1C );
            adr.func = ATT_SBADDH_FUNC( win->reg_1C );
            adr.sba28 = win->sb_address >> 28u;
            adr.sba29 = win->sb_address >> 29u;
            //hwif_sb_read( adr, addr - base, buffer, count );
            return 0;
        }
        return i;
    } else
        return -1;
}

int att_sb_write( att_inst *att, int addr, const void *buffer, int count, int sai ) {
    int base,i;
    att_sb_window *win;
    win = att_find_sb_win( att, addr );
    if ( win ) {
        base = win->window_base;
        i = sb_write(
                        ATT_SBADDR_ENDPT( win->sb_address ),
                        ATT_SBADDR_WROP( win->sb_address ),
                        ATT_SBADDR_BAR( win->sb_address ),
                        addr - base, buffer, count, sai );
        if ( i == -2 ) {
            hwif_sbaddr adr;
            adr.endpt = ATT_SBADDR_ENDPT(win->sb_address);
            adr.bar = ATT_SBADDR_BAR( win->sb_address );
            adr.rd_op = ATT_SBADDR_RDOP( win->sb_address );
            adr.wr_op = ATT_SBADDR_WROP( win->sb_address );
            adr.rs = ATT_SBADDH_RS( win->reg_1C );
            adr.func = ATT_SBADDH_FUNC( win->reg_1C );
            adr.sba28 = win->sb_address >> 28u;
            adr.sba29 = win->sb_address >> 29u;
            //hwif_sb_write( adr, addr - base, buffer, count );
            return 0;
        }
        return i;
    } else
        return -1;
}