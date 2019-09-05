//
// Created by pbx on 15/04/19.
//
#include "tracehub.h"
#include <stdint.h>
#include "log.h"
#include "printf.h"

void tracehub_fake_probe( thub_inst *t ) {
    t->gth_scrpd0 = 1 << 24;
    t->gth_swdest[2] = 0x80000 | 0x8000 | 0x800 | 0x80 | 0x8;
}

void tracehub_gth_read( thub_inst *t, uint32_t addr, void *buffer, int count) {
    uint32_t *p32 = buffer;
    int i;
    if ( addr & 3u || count != 4 ) {
        log(LOG_ERROR, t->self.name, "Non-aligned read to GTH: off: %05x cnt: %05x\n",
                addr,
                count);
        return;
    }
    if ( addr == 0xE0 ) {
        *p32 = t->gth_scrpd0;
        log(LOG_DEBUG, t->self.name, "Read SCRPD0: 0x%08x", t->gth_scrpd0);

    }
    else if ( addr >= 0x8  && addr <= 0x84 ) {
        *p32 = t->gth_swdest[(addr - 8) / 4];
    }
}

void tracehub_gth_write( thub_inst *t, uint32_t addr, const void *buffer, int count) {
    const uint32_t *p32 = buffer;
    if ( addr & 3u || count != 4 ) {
        log(LOG_ERROR, t->self.name, "Non-aligned write to GTH: off: %05x cnt: %05x\n",
                   addr,
                   count);
        return;
    }
    if ( addr == 0xE0 )
        t->gth_scrpd0 = *p32;
    else if ( addr >= 0x8  && addr <= 0x84 ) {
        t->gth_swdest[(addr - 8) / 4] = *p32;
    }

}