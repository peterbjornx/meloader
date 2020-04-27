//
// Created by pbx on 08/08/19.
//

#include "misa/emu.h"
#include "devreg.h"
#include <stdlib.h>
#include <string.h>
#include "log.h"


void misa_process(device_instance *inst ) {
    misa_inst *sa = inst->impl;
    misa_sram_process( sa );
}

device_instance * misa_spawn(const cfg_file *file, const cfg_section *section) {
    misa_inst *i = malloc( sizeof(misa_inst) );

    if ( !i ) {
        log( LOG_FATAL, section->name, "Could not allocate MISA instance structure" );
        exit(EXIT_FAILURE);
    }
    memset( i, 0, sizeof(misa_inst) );

    i->bunit_user = 0;
    cfg_find_int32( section, "bunit_usermode", (uint32_t *) &i->bunit_user );

    i->self.impl = i;
    i->self.name = section->name;

    misa_aunit_bus_init( i, section );
    misa_aunit_bus_target_init( i, section );
    misa_fsb_init( i, section );
    misa_aunit_iommu_init( i, section );
    misa_sram_init( i, section );
    misa_rom_init( i, section );
    i->self.process = misa_process;
    device_register( &i->self );

    return &i->self;
}

device_type misa_type = {
        .name = "misa",
        .spawn = misa_spawn
};

static __attribute__((constructor)) void register_misa() {
    device_type_register( &misa_type );
}