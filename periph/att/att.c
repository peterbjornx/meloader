//
// Created by pbx on 15/04/19.
//
#include "att.h"
#include "meloader.h"
#include "printf.h"
#include "sideband.h"

#define ATT_BASE (0xF00A8000);
#define ATT_SIZE (0x1000)

int att_read(int addr, void *buffer, int count ) {
    uint32_t *buf = buffer;
    addr -= ATT_BASE;
    if ( addr < 0 || addr >= ATT_SIZE )
        return 0;
    mel_printf("[att ] read  unknown 0x%03x count:%i val: 0x%08x\n",
            addr, count, *buf);
    return 1;
}

int att_write( int addr, const void *buffer, int count ) {
    const uint32_t *buf = buffer;
    addr -= ATT_BASE;
    if ( addr < 0 || addr >= ATT_SIZE )
        return 0;
    mel_printf("[att ] write unknown 0x%03x count:%i val: 0x%08x\n",
               addr, count, *buf);
    return 1;
}

mmio_periph att_mmio = {
        .write = att_write,
        .read = att_read
};

void att_sbb_install();
void att_bus_install();

void att_install() {
    att_sbb_install();
    att_bus_install();
    krnl_periph_reg( &att_mmio );
}

