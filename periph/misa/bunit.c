//
// Created by pbx on 19/07/19.
//

#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include "misa/emu.h"
#include "log.h"

int misa_is_bunit_addr( misa_inst *sa, uint32_t addr ) {
    return
            ( (addr & MISA_HEC_ECBASE_MASK) !=
              (sa->registers.hunit.HEC & MISA_HEC_ECBASE_MASK) &&
              (
                      (addr & MISA_HMBOUND_HMBOUND_MASK) <
                      (sa->registers.hunit.HMBOUND & MISA_HMBOUND_HMBOUND_MASK) ||

                      (addr & MISA_HROMMB_ROMBASE_MASK) >=
                      (sa->registers.hunit.HROMMB) ) );
}

void misa_bunit_upstream_write( misa_inst *sa, uint32_t addr, const void *buffer, size_t size )
{
    uint32_t rom_base = sa->registers.hunit.HROMMB & MISA_HROMMB_ROMBASE_MASK;
    if ( sa->bunit_user ) {
        memcpy( ( void * ) addr, buffer, size );
    } else if ( addr < sa->sram_size ) {
        misa_sram_write( sa, addr, buffer, size );
    } else if ( addr >= rom_base ) {
        misa_rom_write( sa, addr - rom_base, buffer, size );
    } else
        log( LOG_ERROR, sa->self.name, "bunit write did not hit either SRAM or ROM! Addr: %08x", addr );
}

void misa_bunit_upstream_read( misa_inst *sa, uint32_t addr,        void *buffer, size_t size )
{
    uint32_t rom_base = sa->registers.hunit.HROMMB & MISA_HROMMB_ROMBASE_MASK;
    if ( sa->bunit_user ) {
        memcpy( buffer, ( void * ) addr, size );
    } else if ( addr < sa->sram_size ) {
        misa_sram_read( sa, addr, buffer, size );
    } else if ( addr >= rom_base ) {
        misa_rom_read( sa, addr - rom_base, buffer, size );
    } else
        log( LOG_ERROR, sa->self.name, "bunit read did not hit either SRAM or ROM! Addr: %08x", addr );
}