//
// Created by pbx on 1/09/19.
//
#include <string.h>
#include <stdlib.h>
#include "pci/bus.h"
#include "pci/device.h"
#include "devreg.h"
#include "pmc.h"
#include "log.h"
#include <stdio.h>
#include <pmc.h>

static int pmc_bar_read(pci_func *func, int bar, uint64_t addr, void *buffer, int count) {
    pmc_inst *t = func->device->impl;
    if ( bar == 0 ) {
        if ( addr == 0x020 && count == 4 ) {
            *(uint32_t *)buffer = t->d31id;
        } else if ( addr == 0x310 && count == 4 ) {
            *(uint32_t *)buffer = 0x8;
        } else
            log(LOG_ERROR, t->self.name, "Read to unimplemented register %08x size %i", (uint32_t) addr, count);
        return t->sai;
    } else if ( bar == 1 ) {
        log(LOG_ERROR, t->self.name, "Read to unimplemented XRAM %08x size %i", (uint32_t) addr, count);
        return t->sai;

    } else
        log(LOG_ERROR, t->self.name, "Read to unimplemented bar");
    return -1;
}

static int pmc_bar_write(pci_func *func, int bar, uint64_t addr, const void *buffer, int count) {
    pmc_inst *t = func->device->impl;
    if ( bar == 0 ) {
        log(LOG_ERROR, t->self.name, "Write to unimplemented register %08x size %i", (uint32_t) addr, count);
        return t->sai;
    } else
        log(LOG_ERROR, t->self.name, "Write to unimplemented bar");
    return -1;
}

void pmc_config( pmc_inst *pmc, const cfg_section *section ) {
    cfg_find_int32( section, "sai", &pmc->sai );
    cfg_find_int32( section, "d31id", &pmc->d31id );
}

static device_instance * pmc_spawn(const cfg_file *file, const cfg_section *section) {
    uint32_t fp = 0;
    pmc_inst *i = malloc( sizeof(pmc_inst) );

    if ( !i ) {
        log( LOG_FATAL, section->name, "Could not allocate PMC instance structure" );
        exit(EXIT_FAILURE);
    }
    memset( i, 0, sizeof(pmc_inst) );

    i->self.impl = i;
    i->self.name = section->name;
    i->func.bar_size[0] = 0x00002000;
    i->func.bar_size[1] = 0x00004000;
    i->func.max_bar = 2;
    i->func.func.device = &i->self;
    i->func.flags = 0;
    i->func.bar_read = pmc_bar_read;
    i->func.bar_write = pmc_bar_write;

    pmc_config( i, section );

    pci_simple_init( &i->func, section );

    device_register( &i->self );

    return &i->self;
}

device_type pmc_type = {
        .name = "pmc",
        .spawn = pmc_spawn
};

static __attribute__((constructor)) void register_pmc() {
    device_type_register( &pmc_type );
}