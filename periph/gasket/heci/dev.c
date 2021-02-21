//
// Created by pbx on 01/09/19.
//

//
// Created by pbx on 1/09/19.
//
#include <string.h>
#include <stdlib.h>
#include "pci/bus.h"
#include "pci/device.h"
#include "devreg.h"
#include "log.h"
#include <stdio.h>
#include <hwif.h>
#include "gasket/heci/heci.h"

static int heci_bar_read(pci_func *func, int bar, uint64_t addr, void *buffer, int count) {
    heci_inst *t = func->device->impl;
    uint32_t *val;
    if ( bar == 0 ) {
        /*hwif_mm_read(func->config.type0.bar[bar] + addr, buffer, count);
        return t->sai;*/
        if ( count != 4 ) {
            log(LOG_ERROR, t->self.name, "Read to register %08x with bad size %i", (uint32_t) addr, count);
            return t->sai;
        }
        val = (uint32_t *) buffer;
        if ( addr == 0x4 ) {
            *val = t->cse_fs;
            log(LOG_TRACE, t->self.name, "Read FS: %08x", *val);
        } else if ( addr == 0x10 ) {
            *val = t->cse_gs1;
            log(LOG_TRACE, t->self.name, "Read GS1: %08x", *val);
        } else if ( addr == 0x14 ) {
            *val = t->cse_gs2;
            log(LOG_TRACE, t->self.name, "Read GS2: %08x", *val);
        } else if ( addr == 0x18 ) {
            *val = t->cse_gs3;
            log(LOG_TRACE, t->self.name, "Read GS3: %08x", *val);
        } else if ( addr == 0x1C ) {
            *val = t->cse_gs4;
            log(LOG_TRACE, t->self.name, "Read GS4: %08x", *val);
        } else if ( addr == 0x20 ) {
            *val = t->cse_gs5;
            log(LOG_TRACE, t->self.name, "Read GS5: %08x", *val);
        } else if ( addr == 0x24 ) {
            *val = t->cse_shdw1;
            log(LOG_DEBUG, t->self.name, "Read SHDW1: %08x", *val);
        } else if ( addr == 0x28 ) {
            *val = t->cse_shdw2;
            log(LOG_DEBUG, t->self.name, "Read SHDW2: %08x", *val);
        } else if ( addr == 0x5C ) {
            *val = t->cse_cuba;
            log(LOG_DEBUG, t->self.name, "Read CUBA: %08x", *val);
        } else
            log(LOG_ERROR, t->self.name, "Read to unimplemented register %08x size %i", (uint32_t) addr, count);
        return t->sai;
    } else
        log(LOG_ERROR, t->self.name, "Read to unimplemented bar");
    return -1;
}

static int heci_bar_write(pci_func *func, int bar, uint64_t addr, const void *buffer, int count) {
    uint32_t val;
    heci_inst *t = func->device->impl;
    if ( bar == 0 ) {
        /*hwif_mm_write(func->config.type0.bar[bar] + addr, buffer, count);
        if ( count != 4 ) {
            log(LOG_ERROR, t->self.name, "Write to register %08x with bad size %i", (uint32_t) addr, count);
            return t->sai;
        }*/
        val = *(uint32_t *) buffer;
        if ( addr == 0x4 ) {
            log(LOG_TRACE, t->self.name, "Write FS: %08x", val);
            heci_handle_fs_change(t, val);
            t->cse_fs = val;
        } else if ( addr == 0x10 ) {
            log(LOG_TRACE, t->self.name, "Write GS1: %08x", val);
            heci_handle_gs1_change(t, val);
            t->cse_gs1 = val;
        } else if ( addr == 0x14 ) {
            log(LOG_DEBUG, t->self.name, "Write GS2: %08x", val);
            t->cse_gs2 = val;
        } else if ( addr == 0x18 ) {
            log(LOG_DEBUG, t->self.name, "Write GS3: %08x", val);
            t->cse_gs3 = val;
        } else if ( addr == 0x1C ) {
            log(LOG_DEBUG, t->self.name, "Write GS4: %08x", val);
            t->cse_gs4 = val;
        } else if ( addr == 0x20 ) {
            log(LOG_DEBUG, t->self.name, "Write GS5: %08x", val);
            t->cse_gs5 = val;
        } else if ( addr == 0x24 ) {
            log(LOG_DEBUG, t->self.name, "Write SHDW1: %08x", val);
            t->cse_shdw1 = val;
        } else if ( addr == 0x28 ) {
            log(LOG_DEBUG, t->self.name, "Write SHDW2: %08x", val);
            t->cse_shdw2 = val;
        } else if ( addr == 0x5C ) {
            log(LOG_DEBUG, t->self.name, "Write CUBA: %08x", val);
            t->cse_cuba = val;
        } else
            log(LOG_ERROR, t->self.name, "Write to unimplemented register %08x size %i", (uint32_t) addr, count);
        return t->sai;
    } else
        log(LOG_ERROR, t->self.name, "Write to unimplemented bar");
    return -1;
}

void heci_config( heci_inst *pmc, const cfg_section *section ) {
    cfg_find_int32( section, "sai", &pmc->sai );
}

static device_instance * heci_spawn(const cfg_file *file, const cfg_section *section) {
    uint32_t fp = 0;
    heci_inst *i = malloc( sizeof(heci_inst) );

    if ( !i ) {
        log( LOG_FATAL, section->name, "Could not allocate HECI instance structure" );
        exit(EXIT_FAILURE);
    }
    memset( i, 0, sizeof(heci_inst) );

    i->self.impl = i;
    i->self.name = section->name;
    i->func.bar_size[0] = 0x00006000;
    i->func.max_bar = 1;
    i->func.func.device = &i->self;
    i->func.flags = 0;
    i->func.bar_read = heci_bar_read;
    i->func.bar_write = heci_bar_write;
    heci_config( i, section );

    pci_simple_init( &i->func, section );

    device_register( &i->self );

    return &i->self;
}

device_type heci_type = {
        .name = "heci",
        .spawn = heci_spawn
};

static __attribute__((constructor)) void register_heci() {
    device_type_register( &heci_type );
}