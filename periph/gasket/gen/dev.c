//
// Created by pbx on 01/09/19.
//
#include <string.h>
#include <stdlib.h>
#include "pci/bus.h"
#include "pci/device.h"
#include "devreg.h"
#include "log.h"
#include <stdio.h>
#include "gasket/gen/dev.h"

static int gen_bar_read(pci_func *func, int bar, uint64_t addr, void *buffer, int count) {
    gen_inst *t = func->device->impl;
    uint32_t *val = buffer;
    if ( bar == 0 ) {
        if ( count != 4 ) {
            log(LOG_ERROR, t->self.name, "Read from register %08x with bad size %i", (uint32_t) addr, count);
            return t->sai;
        }
        switch ( addr ) {
            case 0x100:
                *val = 0x100000; //TODO: Derive this from command reg (this is probably a RUN/BUSY bit)
                log(LOG_INFO, t->self.name, "Read to register GEN_100: %08x", *(uint32_t *)buffer);
                break;
            case 0x104:
                *val = 0x40C0003;
                log(LOG_INFO, t->self.name, "Read to register GEN_104: %08x", *(uint32_t *)buffer);
                break;
            case 0x108:
                *val = 0x4;
                log(LOG_INFO, t->self.name, "Read to register GEN_108: %08x", *(uint32_t *)buffer);
                break;
            case 0x200:
                *val = 3u << 16u;
                log(LOG_INFO, t->self.name, "Read to register GEN_200: %08x", *(uint32_t *)buffer);
                break;
            case 0x204:
                *val = 0x3F;
                log(LOG_INFO, t->self.name, "Read to register GEN_204: %08x", *(uint32_t *)buffer);
                break;
            case 0x20C:
                *val = (0u << 0u) | (1 << 1u) | (1u << 21u);
                log(LOG_INFO, t->self.name, "Read to register GEN_20C: %08x", *(uint32_t *)buffer);
                break;
            default:
                log(LOG_INFO, t->self.name, "Read to register %08x: %08x", (uint32_t) addr, *(uint32_t *)buffer);
        }

        return t->sai;

    } else if ( bar == 1 ) {
        if ( count != 4 ) {
            log(LOG_ERROR, t->self.name, "Read from register %08x with bad size %i", (uint32_t) addr, count);
            return t->sai;
        }
        switch ( addr ) {
            case 0x50:
                *val = 0x29;
                log(LOG_INFO, t->self.name, "Read to register GEN_1050: %08x", *(uint32_t *)buffer);
                break;
            default:
                log(LOG_INFO, t->self.name, "Read to register 1%03x: %08x", (uint32_t) addr, *(uint32_t *)buffer);
        }

        return t->sai;

    } else
        log(LOG_ERROR, t->self.name, "Read to unimplemented bar");
    return -1;
}

static int gen_bar_write(pci_func *func, int bar, uint64_t addr, const void *buffer, int count) {
    uint32_t val;
    gen_inst *t = func->device->impl;
    if ( bar == 0 ) {
        if ( count != 4 ) {
            log(LOG_ERROR, t->self.name, "Write to register %08x with bad size %i", (uint32_t) addr, count);
            return t->sai;
        }
        log(LOG_INFO, t->self.name, "Write to register %08x: %08x", (uint32_t) addr, *(uint32_t *)buffer);
        return t->sai;
    } else
        log(LOG_ERROR, t->self.name, "Write to unimplemented bar");
    return -2;
}

void gen_config( gen_inst *enclave, const cfg_section *section ) {
    cfg_find_int32( section, "sai", &enclave->sai );
}

static device_instance * gen_spawn(const cfg_file *file, const cfg_section *section) {
    uint32_t fp = 0;
    gen_inst *i = malloc( sizeof(gen_inst) );

    if ( !i ) {
        log( LOG_FATAL, section->name, "Could not allocate gen instance structure" );
        exit(EXIT_FAILURE);
    }
    memset( i, 0, sizeof(gen_inst) );

    i->self.impl = i;
    i->self.name = section->name;
    i->func.bar_size[0] = 0x00001000;
    i->func.bar_size[1] = 0x00001000;
    i->func.bar_size[2] = 0x00001000;
    i->func.bar_size[3] = 0x00001000;
    i->func.bar_size[4] = 0x00001000;
    i->func.max_bar = 5;
    i->func.func.device = &i->self;
    i->func.flags = 0;
    i->func.bar_read = gen_bar_read;
    i->func.bar_write = gen_bar_write;
    gen_config( i, section );

    pci_simple_init( &i->func, section );

    device_register( &i->self );

    return &i->self;
}

device_type gen_type = {
        .name = "gen",
        .spawn = gen_spawn
};

static __attribute__((constructor)) void register_gen() {
    device_type_register( &gen_type );
}