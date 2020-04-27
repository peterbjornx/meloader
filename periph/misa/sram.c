//
// Created by pbx on 02/09/19.
//
#include <string.h>
#include <stdlib.h>
#include "log.h"
#include "misa/emu.h"

//TODO: Properly emulate SRAM bank enables

void misa_sram_process(misa_inst *sa) {
    if ( sa->registers.sram.SRAM_POWER_CTL !=
         sa->registers.sram.SRAM_POWER_STS ) {
        log( LOG_INFO, sa->self.name,
             "SRAM bank power gating change: %08X to %08X",
             sa->registers.sram.SRAM_POWER_STS,
             sa->registers.sram.SRAM_POWER_CTL );
        sa->registers.sram.SRAM_POWER_STS = sa->registers.sram.SRAM_POWER_CTL;
        sa->registers.sram.SRAM_INIT_STS &= ~sa->registers.sram.SRAM_POWER_STS;
    }
    if ( sa->registers.sram.SRAM_INIT_CTL ) {
        log( LOG_INFO, sa->self.name,
             "SRAM bank init request: %08X",
             sa->registers.sram.SRAM_INIT_CTL );
        //TODO: Actually zero the bank(s)
        sa->registers.sram.SRAM_INIT_STS |= sa->registers.sram.SRAM_INIT_CTL;
        sa->registers.sram.SRAM_INIT_CTL = 0;
    }
    if ( sa->registers.sram.SRAM_SM_STS != sa->registers.sram.SRAM_SM_CTL ) {
        log( LOG_INFO, sa->self.name,
             "SRAM bank SM (sleep?) change: %08X to %08X",
             sa->registers.sram.SRAM_SM_STS,
             sa->registers.sram.SRAM_SM_CTL );
        //TODO: Actually zero the bank(s)
        sa->registers.sram.SRAM_SM_STS = sa->registers.sram.SRAM_SM_CTL;
    }
}

void misa_sram_write( misa_inst *sa, uint32_t addr, const void *buffer, size_t size ) {
    if ( addr > sa->sram_size || ( addr + size ) > sa->sram_size ) {
        log( LOG_ERROR, sa->self.name, "SRAM write beyond bounds at %08x", addr );
        return;
    }
    memcpy( sa->sram_data + addr, buffer, size );
}

void misa_sram_read ( misa_inst *sa, uint32_t addr,       void *buffer, size_t size ) {
    if ( addr > sa->sram_size || ( addr + size ) > sa->sram_size ) {
        log( LOG_ERROR, sa->self.name, "SRAM read beyond bounds at %08x", addr );
        return;
    }
    memcpy( buffer, sa->sram_data + addr, size );
}

void misa_sram_init( misa_inst *sa, const cfg_section *section ) {
    int status;

    if ( sa->bunit_user )
        return;

    status = cfg_find_int32(section, "sram_size", &sa->sram_size);
    logassert( status >= 0, section->name, "Missing or invalid SRAM size" );

    sa->sram_data = malloc( sa->sram_size );
    logassert( sa->sram_data != NULL, section->name, "Could not allocate SRAM" );

    memset( sa->sram_data, 0, sa->sram_size ); //TODO: Support randomizing RAM content

    sa->registers.sram.SRAM_FS = 1;
}
