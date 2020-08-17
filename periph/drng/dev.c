//
// Created by pbx on 29/04/20.
//

//
// Created by pbx on 01/09/19.
//
#include <string.h>
#include <stdlib.h>
#include "log.h"
#include "devreg.h"
#include "sideband.h"

typedef struct {
    device_instance self;
    sideband_dev sb;
    int sai;
    uint64_t RNDBUF;
    uint32_t STATUS;
    uint32_t CONFIG;
    uint32_t UNK_18;
    
} drng_inst;

int drng_read( sideband_dev *dev, int bar, int op, int offset, void *buffer, int count, int sai ) {
    drng_inst *i = dev->device->impl;
    if ( op != 6 ) {
        log(LOG_ERROR, dev->device->name, "Invalid sideband opcode %i in read", op);
        return -2;
    }
    if ( bar != 0 ) {
        log(LOG_ERROR, dev->device->name, "Invalid sideband BAR %i in read", bar);
        return -2;
    }
    log(LOG_TRACE, dev->device->name, "Try read drng offset %X size %i", offset, count);
    if ( offset >= 0 && offset + count < 0x8 ) {
        i->RNDBUF = random();
        memcpy(buffer, ((void *)&i->RNDBUF) + offset, count);
    } else if ( offset == 8 && count == 4 ) {
        *(uint32_t *)buffer = i->STATUS;
    } else if ( offset == 0xC && count == 4 ) {
        *(uint32_t *)buffer = i->CONFIG;
    } else if ( offset == 0x18 && count == 4 ) {
        *(uint32_t *)buffer = i->UNK_18;
    } else {
        log(LOG_ERROR, dev->device->name, "Try read drng offset %X size %i", offset, count);
    }
    return i->sai;
}

int drng_write( sideband_dev *dev, int bar, int op, int offset, const void *buffer, int count, int sai ) {
    drng_inst *i = dev->device->impl;
    if ( op != 7 ) {
        log( LOG_ERROR, dev->device->name, "Invalid sideband opcode %i in write", op );
        return -2;
    }
    if ( bar != 0 ) {
        log(LOG_ERROR, dev->device->name, "Invalid sideband BAR %i in write", bar);
        return -2;
    }
    log(LOG_TRACE, dev->device->name, "Try write drng offset %X size %i", offset, count);
    if ( offset == 8 && count == 4 ) {
        i->STATUS = *(uint32_t *)buffer;
    } else if ( offset == 0xC && count == 4 ) {
        i->CONFIG = *(uint32_t *)buffer;
    } else if ( offset == 0x18 && count == 4 ) {
        i->UNK_18 = *(uint32_t *)buffer;
    } else {
        log(LOG_ERROR, dev->device->name, "Try write drng offset %X size %i", offset, count);
    }
    return i->sai;

}

void drng_process( device_instance *inst ) {
    drng_inst *i = inst->impl;
    if ( i->CONFIG != 0 )
        i->STATUS = 0xF;
}

device_instance * drng_spawn(const cfg_file *file, const cfg_section *section) {
    int s;
    drng_inst *i = malloc( sizeof(drng_inst) );
    logassert( i != NULL, section->name, "Could not allocate instance structure" );
    memset( i, 0, sizeof(drng_inst) );
    i->self.name = section->name;
    i->self.impl = i;
    i->self.process = drng_process;

    s = cfg_find_int32(section, "endpoint", (uint32_t *) &i->sb.endpoint);
    logassert( s >= 0, section->name, "No sideband endpoint specified" );

    s = cfg_find_int32(section, "sai", (uint32_t *) &i->sai);
    logassert( s >= 0, section->name, "No SAI specified" );

    i->sb.device = &i->self;
    i->sb.read = drng_read;
    i->sb.write = drng_write;

    sb_register( &i->sb );
    device_register( &i->self );
    return &i->self;
}

device_type drng_type = {
        .name = "drng",
        .spawn = drng_spawn
};

static __attribute__((constructor)) void register_drng() {
    device_type_register( &drng_type );
}
