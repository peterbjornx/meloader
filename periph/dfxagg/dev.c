//
// Created by pbx on 01/09/19.
//
#include <string.h>
#include <stdlib.h>
#include "log.h"
#include "dfxagg.h"
#include "devreg.h"
#include "sideband.h"


void dfxagg_process( dfxagg_inst *i ) {
}

int dfxagg_read( sideband_dev *dev, int bar, int op, int offset, void *buffer, int count, int sai ) {
    dfxagg_inst *i = dev->device->impl;
    if ( op != 6 ) {
        log(LOG_ERROR, dev->device->name, "Invalid sideband opcode %i in read", op);
        return -2;
    }
    if ( bar != 0 ) {
        log(LOG_ERROR, dev->device->name, "Invalid sideband BAR %i in read", bar);
        return -2;
    }
    if (count == 4 && offset == 0x0000) {
        *(uint32_t *) buffer = i->personality;
        log(LOG_TRACE, dev->device->name, "Read personality register: %08X", i->personality);
    } else if (count == 4 && offset == 0x0004) {
        *(uint32_t *) buffer = i->consent;
        log(LOG_TRACE, dev->device->name, "Read consent register: %08X", i->consent);
    } else if (count == 8 && offset == 0x0008) {
        *(uint64_t *) buffer = i->status;
        log(LOG_TRACE, dev->device->name, "Read status register: %016llX", i->status);
    } else if (count == 8 && offset == 0x0018) {
        *(uint64_t *) buffer = i->puid;
        log(LOG_TRACE, dev->device->name, "Read PUID register: %016llX", i->puid);
    } else
        log(LOG_ERROR, dev->device->name, "Try read control offset %X size %i", offset, count);

    return i->sai;
}

int dfxagg_write( sideband_dev *dev, int bar, int op, int offset, const void *buffer, int count, int sai ) {
    dfxagg_inst *i = dev->device->impl;
    if ( op != 7 ) {
        log( LOG_ERROR, dev->device->name, "Invalid sideband opcode %i in write", op );
        return -2;
    }
    if ( bar != 0 ) {
        log(LOG_ERROR, dev->device->name, "Invalid sideband BAR %i in write", bar);
        return -2;
    }
    if ( count == 4 && offset == 0x0000 ) {
        if ( i->consent & CONSENT_PRIVACY_OPT ) {
            i->personality = *(uint32_t *) buffer;
            log(LOG_DEBUG, dev->device->name, "Write personality register: %08X", i->personality);
        } else
            log(LOG_ERROR, dev->device->name, "Tried to write personality register, but no consent given");
    } else if ( count == 4 && offset == 0x0004 ) {
        if ( ~i->consent & CONSENT_LOCK_PRIVACY_OPT ) {
            i->consent = *(uint32_t *) buffer;
            log(LOG_DEBUG, dev->device->name, "Write consent register: %08X", i->consent);
        } else
            log(LOG_ERROR, dev->device->name, "Tried to write consent register, but it is locked");
    } else {
        log(LOG_ERROR, dev->device->name, "Try write control offset %X size %i", offset, count);
    }
    dfxagg_process( i );
    return i->sai;

}

device_instance * dfxagg_spawn(const cfg_file *file, const cfg_section *section) {
    int s;
    dfxagg_inst *i = malloc( sizeof(dfxagg_inst) );
    logassert( i != NULL, section->name, "Could not allocate instance structure" );
    memset( i, 0, sizeof(dfxagg_inst) );
    i->self.name = section->name;
    i->self.impl = i;

    s = cfg_find_int32(section, "endpoint", (uint32_t *) &i->sb.endpoint);
    logassert( s >= 0, section->name, "No sideband endpoint specified" );

    s = cfg_find_int32(section, "sai", (uint32_t *) &i->sai);
    logassert( s >= 0, section->name, "No SAI specified" );

    s = cfg_find_int64(section, "puid", &i->puid);

    i->sb.device = &i->self;
    i->sb.read = dfxagg_read;
    i->sb.write = dfxagg_write;

    sb_register( &i->sb );
    return &i->self;
}

device_type dfxagg_type = {
        .name = "dfxagg",
        .spawn = dfxagg_spawn
};

static __attribute__((constructor)) void register_dfxagg() {
    device_type_register( &dfxagg_type );
}
