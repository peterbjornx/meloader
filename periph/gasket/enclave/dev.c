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
#include <hwif.h>
#include "gasket/enclave/enclave.h"

static int enclave_bar_read(pci_func *func, int bar, uint64_t addr, void *buffer, int count) {
    enclave_inst *t = func->device->impl;
    uint32_t *val;
    if ( bar == 0 ) {
        //hwif_mm_read(func->config.type0.bar[bar] + addr, buffer, count);
        log(LOG_INFO, t->self.name, "Read to register %08x: %08x", (uint32_t) addr, *(uint32_t *)buffer);
        return t->sai;

    } else
        log(LOG_ERROR, t->self.name, "Read to unimplemented bar");
    return -1;
}

static int enclave_bar_write(pci_func *func, int bar, uint64_t addr, const void *buffer, int count) {
    uint32_t val;
    enclave_inst *t = func->device->impl;
    if ( bar == 0 ) {
        /*hwif_mm_write(func->config.type0.bar[bar] + addr, buffer, count);*/
        if ( count != 4 ) {
            log(LOG_ERROR, t->self.name, "Write to register %08x with bad size %i", (uint32_t) addr, count);
            return t->sai;
        }
      log(LOG_INFO, t->self.name, "Write to register %08x: %08x", (uint32_t) addr, *(uint32_t *)buffer);
        return t->sai;
    } else
        log(LOG_ERROR, t->self.name, "Write to unimplemented bar");
    return -1;
}

void enclave_config( enclave_inst *enclave, const cfg_section *section ) {
    cfg_find_int32( section, "sai", &enclave->sai );
}

static device_instance * enclave_spawn(const cfg_file *file, const cfg_section *section) {
    uint32_t fp = 0;
    enclave_inst *i = malloc( sizeof(enclave_inst) );

    if ( !i ) {
        log( LOG_FATAL, section->name, "Could not allocate enclave instance structure" );
        exit(EXIT_FAILURE);
    }
    memset( i, 0, sizeof(enclave_inst) );

    i->self.impl = i;
    i->self.name = section->name;
    i->func.bar_size[0] = 0x00006000;
    i->func.max_bar = 1;
    i->func.func.device = &i->self;
    i->func.flags = 0;
    i->func.bar_read = enclave_bar_read;
    i->func.bar_write = enclave_bar_write;
    enclave_config( i, section );

    pci_simple_init( &i->func, section );

    device_register( &i->self );

    return &i->self;
}

device_type enclave_type = {
        .name = "enclave",
        .spawn = enclave_spawn
};

static __attribute__((constructor)) void register_enclave() {
    device_type_register( &enclave_type );
}