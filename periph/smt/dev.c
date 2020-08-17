//
// Created by pbx on 1/09/19.
//
#include <string.h>
#include <stdlib.h>
#include "pci/bus.h"
#include "pci/device.h"
#include "devreg.h"
#include "smt.h"
#include "log.h"
#include <stdio.h>
#include <smt.h>

static int smt_bar_read(pci_func *func, int bar, uint64_t addr, void *buffer, int count) {
    smt_inst *t = func->device->impl;
    if ( bar == 0 ) {

        if ( addr == 0x000 && count == 4 ) {
            *(uint32_t *)buffer = t->GR_GCTRL;
        } else if ( addr == 0x048 && count == 4 ) {
            *(uint32_t *)buffer = t->MSTR_MCTRL;
        } else if ( addr == 0x04C && count == 4 ) {
            *(uint32_t *)buffer = t->MSTR_MSTS;
        } else
            log(LOG_ERROR, t->self.name, "Read to unimplemented register %08x size %i", (uint32_t) addr, count);
        return t->sai;
    } else
        log(LOG_ERROR, t->self.name, "Read to unimplemented bar");
    return -1;
}

static int smt_bar_write(pci_func *func, int bar, uint64_t addr, const void *buffer, int count) {
    uint32_t val;
    smt_inst *t = func->device->impl;
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
        if ( addr == 0x00 ) {
            log(LOG_DEBUG, t->self.name, "Write SMT GR_GCTRL: %08x", val);
            t->GR_GCTRL = val;
        } else if ( addr == 0x40 ) {
            log(LOG_DEBUG, t->self.name, "Write SMT MSTR_MDBA: %08x", val);
            t->MSTR_MDBA = val;
        } else if ( addr == 0x44 ) {
            log(LOG_DEBUG, t->self.name, "Write SMT MSTR_MDS: %08x", val);
            t->MSTR_MDS = val;
        } else if ( addr == 0x48 ) {
            log(LOG_DEBUG, t->self.name, "Write SMT MSTR_MCTRL: %08x", val);
            t->MSTR_MCTRL = val;
        }  else
            log(LOG_ERROR, t->self.name, "Write to unimplemented register %08x size %i", (uint32_t) addr, count);
        return t->sai;
    } else
        log(LOG_ERROR, t->self.name, "Write to unimplemented bar");
    return -1;
}

void smt_config( smt_inst *smt, const cfg_section *section ) {
    cfg_find_int32( section, "sai", &smt->sai );
}

void smt_process( device_instance *inst ) {
    smt_inst *i = inst->impl;
    if ( i->GR_GCTRL & SMT_GCTRL_SRST ){
        log(LOG_INFO, i->self.name, "Soft reset!");
        i->GR_GCTRL &= ~SMT_GCTRL_SRST;
    }
    if ( i->MSTR_MCTRL & SMT_MCTRL_SS ) {
        i->MSTR_MCTRL &= ~SMT_MCTRL_SS;
        //TODO: Do transaction, DMA in descriptor!
        if ( i->MSTR_MDS > sizeof i->dma_buf ) {
            log(LOG_ERROR, i->self.name, "Master descriptor size %08X is too large", i->MSTR_MDS);
            i->MSTR_MSTS |= SMT_MSTS_MEIS;
            return;
        }
        int st = pci_bus_mem_read( i->func.func.bus, i->MSTR_MDBA, i->dma_buf, i->MSTR_MDS, i->sai, 15 );
        if ( st <= 0 ){
            log(LOG_ERROR, i->self.name, "Could not DMA read master descriptor at %08X with size %08X: %i", (uint32_t) i->MSTR_MDBA, i->MSTR_MDS, st);
            i->MSTR_MSTS |= SMT_MSTS_MEIS;
            return;
        }
        i->MSTR_MSTS |= SMT_MSTS_IP;
    }
}

static device_instance * smt_spawn(const cfg_file *file, const cfg_section *section) {
    uint32_t fp = 0;
    smt_inst *i = malloc( sizeof(smt_inst) );

    if ( !i ) {
        log( LOG_FATAL, section->name, "Could not allocate SMT instance structure" );
        exit(EXIT_FAILURE);
    }
    memset( i, 0, sizeof(smt_inst) );
    i->self.impl = i;
    i->self.name = section->name;
    i->func.bar_size[0] = 0x00001000;
    i->func.max_bar = 1;
    i->func.func.device = &i->self;
    i->func.flags = 0;
    i->func.bar_read = smt_bar_read;
    i->func.bar_write = smt_bar_write;
    i->self.process = smt_process;
    smt_config( i, section );

    pci_simple_init( &i->func, section );

    device_register( &i->self );

    return &i->self;
}

device_type smt_type = {
        .name = "smt",
        .spawn = smt_spawn
};

static __attribute__((constructor)) void register_smt() {
    device_type_register( &smt_type );
}