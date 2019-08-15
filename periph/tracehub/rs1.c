//
// Created by pbx on 15/04/19.
//
#include <string.h>
#include "user/meloader.h"
#include "log.h"
#include "tracehub.h"

int tracehub_mtb_read( thub_inst *t, int addr, void *buffer, int count ) {
    if ( addr < TRACEHUB_TSCU_OFFSET )
        tracehub_gth_read( t, addr - TRACEHUB_GTH_OFFSET, buffer, count);
    return 1;
}

int tracehub_mtb_write( thub_inst *t, int addr, const void *buffer, int count ) {
    if ( addr < TRACEHUB_TSCU_OFFSET )
        tracehub_gth_write( t, addr - TRACEHUB_GTH_OFFSET, buffer, count);
    return 1;
}

static int thub_bar_read(pci_func *func, int bar, uint64_t addr, void *buffer, int count) {
    thub_inst *t = func->device->impl;
    if ( bar == 0 )
        return tracehub_mtb_read( t, addr, buffer, count );
    else if ( bar == 3 )
        return tracehub_ftmr_read( t, addr, buffer, count );
    else
        return -1;
}

static int thub_bar_write(pci_func *func, int bar, uint64_t addr, const void *buffer, int count) {
    thub_inst *t = func->device->impl;
    if ( bar == 0 )
        return tracehub_mtb_write( t, addr, buffer, count );
    else if ( bar == 3 )
        return tracehub_ftmr_write( t, addr, buffer, count );
    else
        return -1;
}

static device_instance * thub_spawn(const cfg_file *file, const cfg_section *section) {
    uint32_t fp = 0;
    thub_inst *i = malloc( sizeof(thub_inst) );

    if ( !i ) {
        log( LOG_FATAL, section->name, "Could not allocate TraceHub instance structure" );
        exit(EXIT_FAILURE);
    }
    memset( i, 0, sizeof(thub_inst) );

    i->self.impl = i;
    i->self.name = section->name;
    i->func.bar_size[3] = 0x00400000;
    i->func.bar_size[0] = 0x00100000;
    i->func.max_bar = 4;
    i->func.func.device = &i->self;
    i->func.flags = 0;
    i->func.bar_read = thub_bar_read;
    i->func.bar_write = thub_bar_write;

    cfg_find_int32( section, "fake_probe", &fp );

    if ( fp )
        tracehub_fake_probe( i );

    pci_simple_init( &i->func, section );

    device_register( &i->self );

    return &i->self;
}

device_type thub_type = {
        .name = "tracehub",
        .spawn = thub_spawn
};

static __attribute__((constructor)) void register_thub() {
    device_type_register( &thub_type );
}