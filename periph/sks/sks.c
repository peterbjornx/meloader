//
// Created by pbx on 15/04/19.
//
#include "meloader.h"
#include "printf.h"

#define SKS_BASE (0xF510F000);
#define SKS_SIZE (0x1000)

int sks_read(int addr, void *buffer, int count ) {
    uint32_t *buf = buffer;
    addr -= SKS_BASE;
    if ( addr < 0 || addr >= SKS_SIZE )
        return 0;
    mel_printf("[sks ] read  0x%03x count:%i\n", addr, count);
    if ( addr == 0x474 ) {
        *buf = 1;
    }
    return 1;
}

int sks_write( int addr, const void *buffer, int count ) {
    addr -= SKS_BASE;
    if ( addr < 0 || addr >= SKS_SIZE )
        return 0;
    mel_printf("[sks ] write 0x%03x count:%i\n", addr, count);
    return 1;
}


mmio_periph sks_mmio = {
        .write = sks_write,
        .read = sks_read
};


void sks_install() {
    krnl_periph_reg( &sks_mmio );
}