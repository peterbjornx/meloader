//
// Created by pbx on 01/09/19.
//
#include <string.h>
#include <stdlib.h>
#include "log.h"
#include "exi.h"
#include "devreg.h"
#include "sideband.h"


void exi_process( exi_inst *i ) {
}

int exi_read( sideband_dev *dev, int bar, int op, int offset, void *buffer, int count, int sai ) {
    exi_inst *i = dev->device->impl;
    if ( op != 6 ) {
        log(LOG_ERROR, dev->device->name, "Invalid sideband opcode %i in read", op);
        return -2;
    }
    if ( bar != 0 ) {
        log(LOG_ERROR, dev->device->name, "Invalid sideband BAR %i in read", bar);
        return -2;
    }
    if (count == 4 && offset == 0x0000) {
        *(uint32_t *) buffer = i->emecc;
        log(LOG_DEBUG, dev->device->name, "Read emecc register: %08X", i->emecc);
    } else if (count == 4 && offset == 0x0004) {
        *(uint32_t *) buffer = i->ectrl;
        log(LOG_DEBUG, dev->device->name, "Read ectrl register: %08X", i->ectrl);
    } else
        log(LOG_ERROR, dev->device->name, "Try read control offset %X size %i", offset, count);

    return i->sai;
}

int exi_write( sideband_dev *dev, int bar, int op, int offset, const void *buffer, int count, int sai ) {
    exi_inst *i = dev->device->impl;
    if ( op != 7 ) {
        log( LOG_ERROR, dev->device->name, "Invalid sideband opcode %i in write", op );
        return -2;
    }
    if ( bar != 0 ) {
        log(LOG_ERROR, dev->device->name, "Invalid sideband BAR %i in write", bar);
        return -2;
    }
    if ( count == 4 && offset == 0x0000 ) {
        i->emecc = *(uint32_t *) buffer;
        if ( i->emecc & 0x10u )
            log(LOG_DEBUG, dev->device->name, "Write emecc register: %08X (ExI enabled)", i->emecc);
        else
            log(LOG_DEBUG, dev->device->name, "Write emecc register: %08X (ExI disabled)", i->emecc);
    } else if ( count == 4 && offset == 0x0004 ) {
        i->ectrl = *(uint32_t *) buffer;
        log(LOG_DEBUG, dev->device->name, "Write ectrl register: %08X", i->ectrl);
    } else {
        log(LOG_ERROR, dev->device->name, "Try write control offset %X size %i", offset, count);
    }
    exi_process( i );
    return i->sai;

}

device_instance * exi_spawn(const cfg_file *file, const cfg_section *section) {
    int s;
    exi_inst *i = malloc( sizeof(exi_inst) );
    logassert( i != NULL, section->name, "Could not allocate instance structure" );
    memset( i, 0, sizeof(exi_inst) );
    i->self.name = section->name;
    i->self.impl = i;

    s = cfg_find_int32(section, "endpoint", (uint32_t *) &i->sb.endpoint);
    logassert( s >= 0, section->name, "No sideband endpoint specified" );

    s = cfg_find_int32(section, "sai", (uint32_t *) &i->sai);
    logassert( s >= 0, section->name, "No SAI specified" );

    s = cfg_find_int32(section, "ectrl", (uint32_t *) &i->ectrl);

    i->sb.device = &i->self;
    i->sb.read = exi_read;
    i->sb.write = exi_write;

    sb_register( &i->sb );
    return &i->self;
}

device_type exi_type = {
        .name = "exi",
        .spawn = exi_spawn
};

static __attribute__((constructor)) void register_exi() {
    device_type_register( &exi_type );
}
