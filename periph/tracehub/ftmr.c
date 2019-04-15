//
// Created by pbx on 15/04/19.
//
#include "meloader.h"
#include "tracehub.h"
#include "printf.h"
#include "mipitrace.h"

int tracehub_ftmr_read( int addr, void *buffer, int count ) {
    addr -= TRACEHUB_FTMR_BASE;
    if ( addr < 0 || addr >= TRACEHUB_FTMR_SIZE )
        return 0;
    mel_printf("[thub] Read from Firmware Trace Memory Region : %08x", addr);
    return 1;
}

int tracehub_ftmr_msgtype(int a) {

    switch( a ) {
        case 0x00:
            return 0;
            break;
        case 0x08:
            return 1;
            break;
        case 0x10:
            return 2;
            break;
        case 0x18:
            return 3;
            break;
        case 0x20:
            return 4;
            break;
        case 0x28:
            return 5;
            break;
        case 0x30:
            return 6;
            break;
        case 0x34:
            return 7;
            break;
        case 0x38:
            return 8;
            break;
        case 0x3C:
            return 9;
            break;
    }
};


int tracehub_ftmr_write(int addr, const void *buffer, int count ) {
    struct th_msg msg;
    addr -= TRACEHUB_FTMR_BASE;
    if ( addr < 0 || addr >= TRACEHUB_FTMR_SIZE )
        return 0;
    int master_chan = addr / 0x40;
    int master = master_chan / 1024;
    int chan = master_chan % 1024;
    int reg_no = tracehub_ftmr_msgtype(addr % 0x40);
    msg.type = reg_no;
    msg.size = count;
    switch ( count ) {
        case 1:
            msg.d8 = *(uint8_t *) buffer;
            break;
        case 2:
            msg.d16 = *(uint16_t *) buffer;
            break;
        case 4:
            msg.d32 = *(uint32_t *) buffer;
            break;
        case 8:
            msg.d64 = *(uint64_t *) buffer;
            break;
        default:
            mel_printf("[thub] Master %i:%i write to %s with unsupported count %i\n", master,chan, mipi_tmsg_types[reg_no], count);
            return 1;
    }
    trace_msg( master, chan, msg );
    return 1;
}