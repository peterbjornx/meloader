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
#include <hwif.h>

static int pmc_bar_read(pci_func *func, int bar, uint64_t addr, void *buffer, int count) {
    pmc_inst *t = func->device->impl;
    /*if ( 1 == 1 ) {
        hwif_mm_read(func->config.type0.bar[bar] + addr, buffer, count);
        return t->sai;
    }*/
    if ( bar == 0 ) {

        //pmc218 = 0x8000002 pmc_204 = 0x10000
        if ( addr == 0x020 && count == 4 ) {
            *(uint32_t *)buffer = t->d31id;
        } else if ( addr == 0x310 && count == 4 ) {
            *(uint32_t *)buffer = 0x8;
        } else if ( addr == 0x218 && count == 4 ) {
            *(uint32_t *)buffer = 0x8000002;
        } else if ( addr == 0x204 && count == 4 ) {
            *(uint32_t *)buffer = t->pps | 0x10000u;
        } else if ( addr == 0x18 && count == 4 ) {
          *(uint32_t *)buffer = 0x0;

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
    uint32_t val;
    pmc_inst *t = func->device->impl;
    /*if ( 1 == 1 ) {
        hwif_mm_write(func->config.type0.bar[bar] + addr, buffer, count);
        return t->sai;
    }*/
    if ( bar == 0 ) {
        if ( count != 4 ) {
            log(LOG_ERROR, t->self.name, "Write to register %08x with bad size %i", (uint32_t) addr, count);
            return t->sai;
        }
        val = *(uint32_t *) buffer;
        if ( addr == 0x10 ) {
            log(LOG_DEBUG, t->self.name, "Write PMC HESTS0: %08x", val);
        } else if ( addr == 0x14 )  {
            log(LOG_DEBUG, t->self.name, "Write PMC HEISIE0: %08x", val);
        } else if ( addr == 0x18 ) {
            log(LOG_DEBUG, t->self.name, "Write PMC HESTS1: %08x", val);
        } else if ( addr == 0x1C )  {
            log(LOG_DEBUG, t->self.name, "Write PMC HEISIE1: %08x", val);
        } else if ( addr == 0x20 )  {
            log(LOG_ERROR, t->self.name, "Write PMC D31ID: %08x", val);
        } else if ( addr == 0x40 )  {
            log(LOG_DEBUG, t->self.name, "Write PMC HCSTS: %08x", val);
        } else if ( addr == 0x44 )  {
            log(LOG_DEBUG, t->self.name, "Write PMC HCSTS2: %08x", val);
        } else if ( addr == 0x200 )  {
            log(LOG_DEBUG, t->self.name, "Write PMC GENCTL: %08x", val);
        } else if ( addr == 0x204 )  {
            log(LOG_DEBUG, t->self.name, "Write PMC PPS: %08x", val);
        } else if ( addr == 0x208 )  {
            log(LOG_DEBUG, t->self.name, "Write PMC PSIENTR: %08x", val);
        } else if ( addr == 0x20C )  {
            log(LOG_DEBUG, t->self.name, "Write PMC PSIEXIT: %08x", val);
        } else if ( addr == 0x210 )  {
            log(LOG_DEBUG, t->self.name, "Write PMC PSCTRL: %08x", val);
            if ( val & 1u ) {
                log(LOG_INFO, t->self.name, "Request handshake");
                t->pps |= 0x200;
            }
        } else if ( addr == 0x218 )  {
            log(LOG_DEBUG, t->self.name, "Write PMC IEWS: %08x", val);
        } else if ( addr == 0x21C )  {
            log(LOG_DEBUG, t->self.name, "Write PMC IEWE: %08x", val);
        } else if ( addr == 0x220 )  {
            log(LOG_DEBUG, t->self.name, "Write PMC IEW2IE: %08x", val);
        } else if ( addr == 0x250 )  {
            log(LOG_DEBUG, t->self.name, "Write PMC SUSPMCFG: %08x", val);
        } else if ( addr == 0x254 )  {
            log(LOG_DEBUG, t->self.name, "Write PMC DSWPMCFG: %08x", val);
        } else if ( addr == 0x260 )  {
            log(LOG_DEBUG, t->self.name, "Write PMC RTCPMCFG: %08x", val);
        } else if ( addr == 0x270 )  {
            log(LOG_DEBUG, t->self.name, "Write PMC EPMMISC: %08x", val);
        } else if ( addr == 0x300 )  {
            log(LOG_DEBUG, t->self.name, "Write PMC GENPMCCOM: %08x", val);
        } else if ( addr == 0x308 )  {
            log(LOG_DEBUG, t->self.name, "Write PMC PMCINTSTS: %08x", val);
        } else if ( addr == 0x30C )  {
            log(LOG_DEBUG, t->self.name, "Write PMC PMCINTEN: %08x", val);
        } else
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
    //hwif_connect("127.0.0.1",4284);
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