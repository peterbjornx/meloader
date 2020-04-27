//
// Created by pbx on 19/07/19.
//
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include "pci/bus.h"
#include "misa/regs.h"
#include "misa/emu.h"
#include "log.h"



void misa_fsb_mem_write( device_instance *dev, uint32_t addr, const void *buffer, size_t size )
{

    misa_inst *sa = dev->impl;

    if ( misa_is_bunit_addr( sa, addr ) ) {
        misa_bunit_upstream_write( sa, addr, buffer, size );
    } else {
        misa_aunit_upstream_write( sa, addr, buffer, size );
    }

}

void misa_fsb_mem_read( device_instance *dev, uint32_t addr,       void *buffer, size_t size )
{

    misa_inst *sa = dev->impl;

    if ( misa_is_bunit_addr( sa, addr ) ) {
        misa_bunit_upstream_read ( sa, addr, buffer, size );
    } else {
        misa_aunit_upstream_read ( sa, addr, buffer, size );
    }

}

void misa_fsb_io_write( device_instance *dev, uint32_t addr, const void *buffer, size_t size )
{

    misa_inst *sa = dev->impl;

    /* HMBOUND should always be higher than 64K so this is an easy hack to
     * provide IO ports */
    misa_bunit_upstream_write( sa, addr & 0xFFFFU, buffer, size );

}

void misa_fsb_io_read( device_instance *dev, uint32_t addr,       void *buffer, size_t size )
{

    misa_inst *sa = dev->impl;

    /* HMBOUND should always be higher than 64K so this is an easy hack to
     * provide IO ports */
    misa_bunit_upstream_read ( sa, addr, buffer, size );

}

void misa_fsb_init( misa_inst *sa, const cfg_section *section ) {
    const char *cpu;

    cpu = cfg_find_string( section, "cpu" );
    if (! cpu ) {
        log( LOG_FATAL, section->name, "Missing or invalid CPU device name" );
        exit( EXIT_FAILURE );
    }

    sa->cpu = device_find( cpu );
    if (! sa->cpu ) {
        log( LOG_FATAL, section->name, "Could not find CPU \"%s\"", cpu );
        exit( EXIT_FAILURE );
    }

    /* FSB structure must be first entry of CPU device impl */
    sa->fsb = sa->cpu->impl;
    sa->fsb->sa = &sa->self;
    sa->fsb->sa_mem_read = misa_fsb_mem_read;
    sa->fsb->sa_mem_write = misa_fsb_mem_write;
    sa->fsb->sa_io_read = misa_fsb_io_read;
    sa->fsb->sa_io_write = misa_fsb_io_write;

    if ( cfg_find_int32( section, "ecbase" , &sa->registers.hunit.HEC ) >= 0 )
        sa->registers.hunit.HEC |= MISA_HEC_ECEN;
    sa->registers.aunit.AEC = sa->registers.hunit.HEC;

    cfg_find_int32( section, "rombase", &sa->registers.hunit.HROMMB );
    cfg_find_int32( section, "hmbound", &sa->registers.hunit.HMBOUND );

}