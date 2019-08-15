//
// Created by pbx on 08/08/19.
//
#include <stdio.h>
#include "pci/bus.h"
#include "misa/regs.h"
#include "misa/emu.h"
#include "log.h"

uint32_t misa_aunit_iommu_translate( misa_inst *sa, uint32_t addr ) {

    uint32_t linpgmap;
    int i;

    /* If the IOMMU is not enabled, we do not need to do anything */
    if ( ~sa->registers.aunit.AIOMCTL & MISA_AIOMCTL_IATEN )
        return addr;

    for ( i = 0; i < 64; i++ ) {
        if ( ~sa->registers.aunit.AIOM[i].LIN & MISA_AIOMLIN_EVLD )
            continue;
        linpgmap = sa->registers.aunit.AIOM[i].LIN & MISA_AIOMLIN_LINPGMAP_MASK;
        if ( linpgmap != (addr & MISA_AIOMLIN_LINPGMAP_MASK) )
            continue;
        return (addr &~MISA_AIOMLIN_LINPGMAP_MASK) |
                (sa->registers.aunit.AIOM[i].PHY & MISA_AIOMLIN_LINPGMAP_MASK);
    }

    //TODO: Implement error log
    log( LOG_ERROR, sa->self.name, "IOMMU Translation miss for address %08X", addr );

    return ~addr;

}

int misa_aunit_may_complete( misa_inst *sa, int sai ) {
    int scmi = sai / 32;
    uint32_t scbi = sai % 32;
    int r;

    if ( scmi >= 8 ) {
        log( LOG_ERROR, sa->self.name, "Invalid SAI for completion: %02X", sai);
        r = 0;
    } else
        r = sa->registers.aunit.ACPLMTX[scmi] & ( 1u << scbi );

    if ( !r ) {
        log( LOG_ERROR, sa->self.name, "Blocked SAI for completion: %02X", sai);
    }

    return r;

}

void misa_aunit_iommu_init( misa_inst *sa, const cfg_section *section ) {
    int i;
    char name[40];
    cfg_find_int32( section, "aiomctl", &sa->registers.aunit.AIOMCTL );
    for ( i = 0; i < 8; i++ ) {
        snprintf( name, 40, "acplmtx%i", i );
        cfg_find_int32( section, name, &sa->registers.aunit.ACPLMTX[i] );
    }
}