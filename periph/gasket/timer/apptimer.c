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
#include <sys/time.h>
#include "gasket/timer/apptimer.h"

static int apptimer_bar_read(pci_func *func, int bar, uint64_t addr, void *buffer, int count) {
    apptimer_inst *t = func->device->impl;
    uint32_t *val;
    if ( bar == 0 ) {
        if ( addr + count >= sizeof t->bar0_regs ) {
            log( LOG_ERROR, t->self.name, "Read out of bounds to %08x with size %08x", (uint32_t) addr, count);
        } else {
            memcpy( buffer, ((void *)&t->bar0_regs) + addr, count );
        }
        log(LOG_TRACE, t->self.name, "Read to register %08x: %08x", (uint32_t) addr, *(uint32_t *)buffer);
        return t->sai;

    } else
        log(LOG_ERROR, t->self.name, "Read to unimplemented bar");
    return -1;
}

static int apptimer_bar_write(pci_func *func, int bar, uint64_t addr, const void *buffer, int count) {
    uint32_t val;
    apptimer_inst *t = func->device->impl;
    if ( bar == 0 ) {
        if ( addr + count >= sizeof t->bar0_regs ) {
            log( LOG_ERROR, t->self.name, "Write out of bounds to %08x with size %08x", (uint32_t) addr, count);
        } else {
            memcpy( ((void *)&t->bar0_regs) + addr, buffer, count );
        }
        log(LOG_TRACE, t->self.name, "Write to register %08x: %08x", (uint32_t) addr, *(uint32_t *)buffer);
        return t->sai;
    } else
        log(LOG_ERROR, t->self.name, "Write to unimplemented bar");
    return -1;
}

void apptimer_process( device_instance *inst ) {
    apptimer_inst *a = inst->impl;
    struct timeval currentTime;
    gettimeofday(&currentTime, NULL);
    long long ts = currentTime.tv_sec * (long long)1e6 + currentTime.tv_usec;
    a->bar0_regs.TIME_NOW = ts*100000;
    if ( a->bar0_regs.TIME_ALARM <= a->bar0_regs.TIME_NOW &&
         a->bar0_regs.CONTROL & 1u ) {
        log( LOG_INFO, inst->name, "RING RING RING!!!");
        a->bar0_regs.CONTROL &= ~1u;
    }
}

void apptimer_config( apptimer_inst *apptimer, const cfg_section *section ) {
    cfg_find_int32( section, "sai", &apptimer->sai );
}

static device_instance * apptimer_spawn(const cfg_file *file, const cfg_section *section) {
    uint32_t fp = 0;
    apptimer_inst *i = malloc( sizeof(apptimer_inst) );

    if ( !i ) {
        log( LOG_FATAL, section->name, "Could not allocate apptimer instance structure" );
        exit(EXIT_FAILURE);
    }
    memset( i, 0, sizeof(apptimer_inst) );
    i->self.process = apptimer_process;
    i->self.impl = i;
    i->self.name = section->name;
    i->func.bar_size[0] = 0x00006000;
    i->func.max_bar = 1;
    i->func.func.device = &i->self;
    i->func.flags = 0;
    i->func.bar_read = apptimer_bar_read;
    i->func.bar_write = apptimer_bar_write;
    apptimer_config( i, section );

    pci_simple_init( &i->func, section );

    device_register( &i->self );

    return &i->self;
}

device_type apptimer_type = {
        .name = "apptimer",
        .spawn = apptimer_spawn
};

static __attribute__((constructor)) void register_apptimer() {
    device_type_register( &apptimer_type );
}