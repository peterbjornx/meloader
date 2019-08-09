//
// Created by pbx on 08/08/19.
//

//
// Created by pbx on 08/08/19.
//

#include "cpu/usercpu.h"
#include "devreg.h"
#include <stdlib.h>
#include <string.h>
#include "log.h"

void usercpu_legacywire( device_instance *inst, int m_int, int m_nmi) {
    log(LOG_ERROR, inst->name, "legacy_wire is not yet implemented");
}

void usercpu_cpuread(device_instance *inst, uint32_t m_int, void *m_nmi, size_t i) {
    log(LOG_ERROR, inst->name, "mia_mem_read is not yet implemented");
}

void usercpu_cpuwrite(device_instance *inst, uint32_t m_int, const void *m_nmi, size_t i) {
    log(LOG_ERROR, inst->name, "mia_mem_read is not yet implemented");
}

device_instance * usercpu_spawn(const cfg_file *file, const cfg_section *section) {
    usercpu_inst *i = malloc( sizeof(usercpu_inst) );

    if ( !i ) {
        log( LOG_FATAL, section->name, "Could not allocate MISA instance structure" );
        exit(EXIT_FAILURE);
    }
    memset( i, 0, sizeof(usercpu_inst) );

    i->self.impl = i;
    i->self.name = section->name;
    i->fsb.cpu = &i->self;
    i->fsb.mia_legacy_wire = usercpu_legacywire;
    i->fsb.mia_mem_read = usercpu_cpuread;
    i->fsb.mia_mem_write = usercpu_cpuwrite;

    device_register( &i->self );

    return &i->self;
}

device_type usercpu_type = {
        .name = "usercpu",
        .spawn = usercpu_spawn
};

static __attribute__((constructor)) void register_usercpu() {
    device_type_register( &usercpu_type );
}