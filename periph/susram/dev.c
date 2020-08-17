//
// Created by pbx on 01/09/19.
//
#include <string.h>
#include <stdlib.h>
#include "log.h"
#include "susram.h"
#include "devreg.h"
#include "sideband.h"

int susram_read( sideband_dev *dev, int bar, int op, int offset, void *buffer, int count, int sai ) {
    susram_inst *i = dev->device->impl;
    if ( op != 6 ) {
        log(LOG_ERROR, dev->device->name, "Invalid sideband opcode %i in read", op);
        return -2;
    }
    if ( bar != 0 ) {
        log(LOG_ERROR, dev->device->name, "Invalid sideband BAR %i in read", bar);
        return -2;
    }
    log(LOG_ERROR, dev->device->name, "Try read susram offset %X size %i", offset, count);

    return i->sai;
}

int susram_write( sideband_dev *dev, int bar, int op, int offset, const void *buffer, int count, int sai ) {
    susram_inst *i = dev->device->impl;
    if ( op != 7 ) {
        log( LOG_ERROR, dev->device->name, "Invalid sideband opcode %i in write", op );
        return -2;
    }
    if ( bar != 0 ) {
        log(LOG_ERROR, dev->device->name, "Invalid sideband BAR %i in write", bar);
        return -2;
    }
    log(LOG_ERROR, dev->device->name, "Try write susram offset %X size %i", offset, count);
    return i->sai;

}

device_instance * susram_spawn(const cfg_file *file, const cfg_section *section) {
    int s;
    susram_inst *i = malloc( sizeof(susram_inst) );
    logassert( i != NULL, section->name, "Could not allocate instance structure" );
    memset( i, 0, sizeof(susram_inst) );
    i->self.name = section->name;
    i->self.impl = i;

    s = cfg_find_int32(section, "endpoint", (uint32_t *) &i->sb.endpoint);
    logassert( s >= 0, section->name, "No sideband endpoint specified" );

    s = cfg_find_int32(section, "sai", (uint32_t *) &i->sai);
    logassert( s >= 0, section->name, "No SAI specified" );

    i->sb.device = &i->self;
    i->sb.read = susram_read;
    i->sb.write = susram_write;

    sb_register( &i->sb );
    return &i->self;
}

device_type susram_type = {
        .name = "susram",
        .spawn = susram_spawn
};

static __attribute__((constructor)) void register_susram() {
    device_type_register( &susram_type );
}
