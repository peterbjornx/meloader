//
// Created by pbx on 19/07/19.
//
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "pci/bus.h"
#include "misa/regs.h"
#include "misa/emu.h"
#include "log.h"

void misa_aunit_upstream_write( misa_inst *sa, uint32_t addr, const void *buffer, size_t size )
{
    if ( (sa->registers.hunit.HEC & MISA_HEC_ECEN) &&
         (addr & MISA_HEC_ECBASE_MASK) ==
         (sa->registers.hunit.HEC & MISA_HEC_ECBASE_MASK) ) {
        pci_bus_config_write( &sa->bus, addr & ~MISA_HEC_ECBASE_MASK, buffer, size );
    } else if ( addr == 0xCF8 ) {
        if ( size > 4 ) {
            log( LOG_ERROR, sa->self.name, "Invalid CF8 IO write size: %i", size );
            size = 4;
        }
        memcpy( &sa->registers.aunit.ACF8, buffer, size );
    } else if ( addr == 0xCFC ) {
        if ( ~sa->registers.aunit.ACF8 & MISA_ACF8_CF8EN ) {
            log( LOG_ERROR, sa->self.name, "Invalid CFC IO write: CF8 not enabled" );
            return;
        }
        if ( size > 8 ) {
            log( LOG_WARN, sa->self.name, "Invalid CFC IO write size: %i", size );
            size = 8;
        }
        pci_bus_config_write( &sa->bus,
                PCI_PACK_ADDR(
                        MISA_ACF8_G_BUS(sa->registers.aunit.ACF8),
                        MISA_ACF8_G_DEV(sa->registers.aunit.ACF8),
                        MISA_ACF8_G_FUNC(sa->registers.aunit.ACF8),
                        MISA_ACF8_G_REGOFF(sa->registers.aunit.ACF8)),
                buffer, size );
    } else if ( addr <= 0xFFFF ) {
        pci_bus_io_write( &sa->bus, addr, buffer, size, sa->sai, 16 );
    } else {
        pci_bus_mem_write( &sa->bus, addr, buffer, size, sa->sai, 16 );
    }
}

void misa_aunit_upstream_read( misa_inst *sa, uint32_t addr,        void *buffer, size_t size )
{
    if ( (sa->registers.hunit.HEC & MISA_HEC_ECEN) &&
         (addr & MISA_HEC_ECBASE_MASK) ==
         (sa->registers.hunit.HEC & MISA_HEC_ECBASE_MASK) ) {
        pci_bus_config_read( &sa->bus, addr & ~MISA_HEC_ECBASE_MASK, buffer, size );

    } else if ( addr == 0xCF8 ) {
        if ( size > 4 ) {
            log( LOG_ERROR, sa->self.name, "Invalid CF8 IO read size: %i", size );
            size = 4;
        }
        memcpy( buffer, &sa->registers.aunit.ACF8, size );
    } else if ( addr == 0xCFC ) {
        if ( ~sa->registers.aunit.ACF8 & MISA_ACF8_CF8EN ) {
            log( LOG_ERROR, sa->self.name, "Invalid CFC IO read: CF8 not enabled" );
            return;
        }
        if ( size > 8 ) {
            log( LOG_WARN, sa->self.name, "Invalid CFC IO read size: %i", size );
            size = 8;
        }
        pci_bus_config_read( &sa->bus,
                PCI_PACK_ADDR(
                    MISA_ACF8_G_BUS(sa->registers.aunit.ACF8),
                    MISA_ACF8_G_DEV(sa->registers.aunit.ACF8),
                    MISA_ACF8_G_FUNC(sa->registers.aunit.ACF8),
                    MISA_ACF8_G_REGOFF(sa->registers.aunit.ACF8)),
                buffer, size );
    } else if ( addr <= 0xFFFF ) {
        pci_bus_io_read( &sa->bus, addr, buffer, size, sa->sai, 16 );
    } else {
        int s = pci_bus_mem_read( &sa->bus, addr, buffer, size, sa->sai, 16 );
        if ( s < 0 || !misa_aunit_may_complete( sa, s ) ) {
            log( LOG_ERROR, sa->self.name, "Bus error while reading from primary bus addr %08X size %8X",
                    addr, size);
            //TODO: Do not load results here
        }
    }

}

void misa_aunit_bus_init( misa_inst *sa, const cfg_section *section ) {
    memset( &sa->bus, 0, sizeof sa->bus );

    sa->bus.name = cfg_find_string( section, "prim_bus" );
    if (! sa->bus.name ) {
        log( LOG_FATAL, section->name, "Missing or invalid primary bus name" );
        exit( EXIT_FAILURE );
    }

    sa->bus.bus_num = 0x100; // Higher than can be signalled, no TYPE0 transactions here
    pci_bus_register( &sa->bus );
}