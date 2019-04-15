//
// Created by pbx on 15/04/19.
//
#include "tracehub.h"
#include <stdint.h>
#include "printf.h"
uint32_t gth_scrpd0 = 1 << 24;
uint32_t gth_swdest[32];

void tracehub_fake_probe() {
    gth_scrpd0 = 1 << 24;
    gth_swdest[2] = 0x80000 | 0x8000 | 0x800 | 0x80 | 0x8;
}

void tracehub_gth_read(int addr, void *buffer, int count) {
    uint32_t *p32 = buffer;
    int i;
    if ( addr & 3 || count != 4 ) {
        mel_printf("[thub] Non-aligned read to GTH: off: %05x cnt: %05x\n",
                addr,
                count);
        return;
    }
    if ( addr == 0xE0 )
        *p32 = gth_scrpd0;
    else if ( addr >= 0x8  && addr <= 0x84 ) {
        *p32 = gth_swdest[(addr - 8) / 4];
    }
}

void tracehub_gth_write(int addr, const void *buffer, int count) {
    const uint32_t *p32 = buffer;
    if ( addr & 3 || count != 4 ) {
        mel_printf("[thub] Non-aligned write to GTH: off: %05x cnt: %05x\n",
                   addr,
                   count);
        return;
    }
    if ( addr == 0xE0 )
        gth_scrpd0 = *p32;
    else if ( addr >= 0x8  && addr <= 0x84 ) {
        gth_swdest[(addr - 8) / 4] = *p32;
    }

}