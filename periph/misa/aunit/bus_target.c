//
// Created by pbx on 08/08/19.
//
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "pci/device.h"
#include "pci/bus.h"
#include "misa/regs.h"
#include "misa/emu.h"
#include "log.h"

static int is_mmio_hit( pci_func *func, uint64_t addr, int *count ) {
    uint32_t off = addr - func->config.type0.bar[0];

    if ( ~func->config.type0.command & PCI_COMMAND_MSE ||
         addr < func->config.type0.bar[0] ||
         off >= sizeof(misa_regs) )
        return 0;

    if ( off + *count > sizeof(misa_regs) ) {

        log(LOG_WARN, func->device->name,
            "MMIO access beyond bounds"
            "( BAR 0, Offset: 0x%03x, Size: 0x%x )", count);

        *count = sizeof(misa_regs) - off;

    }

    return 1;

}

static int is_dma_hit( pci_func *func, uint64_t addr, int *count, int sai, int lat ) {
    misa_inst *sa = func->device->impl;

    /* Subtractive decode from 16 clocks seems like a safe way of handling DMA */
    /* another option would be to check against HMBOUND */
    return lat > 13;

}


static int bus_mem_read( pci_func *func, uint64_t addr,       void *out, int count, int sai, int lat ) {
    uint32_t taddr;
    uint32_t off;
    misa_inst *sa = func->device->impl;
    off = addr - (func->config.type0.bar[0]&~0x1FFFu);

    if ( is_mmio_hit( func, addr, &count ) ) {

        //TODO: Catch unimplemented reads
        memcpy( out, ((void *)&sa->registers) + off, count );
        log(LOG_TRACE, sa->self.name, "read 0x%03x count:%i val: 0x%08x", addr, count, *(uint32_t*)out);

        return sa->sai;

    }

    //TODO: Handle MSIs

    if ( is_dma_hit( func, addr, &count, sai, lat ) ) {

        //TODO: Implement IOMMU Access Control

        taddr = misa_aunit_iommu_translate( sa, addr );

        /* Check for miss. */
        if ( (taddr ^ addr) & 1u )
            return -1;

        misa_bunit_upstream_read( sa, taddr, out, count );

        return sa->sai;
    }

    return -1;

}

static int bus_mem_write( pci_func *func, uint64_t addr, const void *out, int count, int sai, int lat ) {

    uint32_t off;
    uint32_t taddr;
    misa_inst *sa = func->device->impl;
    off = addr - (func->config.type0.bar[0]&~0x1FFFu);

    if ( is_mmio_hit( func, addr, &count ) ) {

        //TODO: Handle write permissions, catch unimplemented writes
        memcpy( ((void *)&sa->registers) + off, out, count );
        log(LOG_TRACE, sa->self.name, "write 0x%03x count:%i val: 0x%08x", addr, count, *(uint32_t*)out);

        return 0;

    }

    //TODO: Handle MSIs

    if ( is_dma_hit( func, addr, &count, sai, lat ) ) {

        //TODO: Implement IOMMU Access Control

        taddr = misa_aunit_iommu_translate( sa, addr );

        /* Check for miss. */
        if ( (taddr ^ addr) & 1u )
            return -1;

        //log(LOG_TRACE, sa->self.name, "DMA write %08x %08x %x", addr, taddr, count);
        misa_bunit_upstream_write( sa, taddr, out, count );

        return sa->sai;

    }

    return -1;

}

static int bus_cfg_read( pci_func *func, uint64_t addr,       void *out, int count )
{
    int off;
    misa_inst *sa;

    /* Check if this is a type 1 transaction */
    if ( PCI_ADDR_TYPE(addr) != PCI_ADDR_TYPE1 )
        return -1;

    sa = func->device->impl;

    /* Because we use type 1 transactions we need to verify our own BDF */
    /* and as our bus does not have the same bus number as our config space */
    /* we cannot use the pci_func bdf field */
    if ( (addr & PCI_ADDR_BDF_MASK) != sa->bdf )
        return -1;

    off = PCI_ADDR_REG(addr);

    /* Validate the target addresses */
    if ( off + count > PCI_CONFIG_SIZE ) {
        log( LOG_ERROR,
             func->device->name,
             "Configuration read beyond bounds ( Offset: 0x%03x, Size: 0x%x )",
             off, count);
        return -1;
    }

    //TODO: Verify config reads
    /* Copy the configuration space contents to the buffer */
    memcpy( out, off + (void *) &func->config, count );

    /* Signal successful completion */
    return 0;

}

static int bus_cfg_write( pci_func *func, uint64_t addr, const void *out, int count )
{
    int off;
    misa_inst *sa;

    /* Check if this is a type 1 transaction */
    if ( PCI_ADDR_TYPE(addr) != PCI_ADDR_TYPE1 )
        return -1;

    sa = func->device->impl;

    /* Because we use type 1 transactions we need to verify our own BDF */
    /* and as our bus does not have the same bus number as our config space */
    /* we cannot use the pci_func bdf field */
    if ( (addr & PCI_ADDR_BDF_MASK) != sa->bdf )
        return -1;

    /* The bus only issues type 0 transactions to the targeted function,
     * so we do not need to verify the BDF part of the address */

    off = PCI_ADDR_REG(addr);

    /* Validate the target addresses */
    if ( off + count > PCI_CONFIG_SIZE ) {
        log( LOG_ERROR,
             func->device->name,
             "Configuration write beyond bounds ( Offset: 0x%03x, Size: 0x%x )",
             off, count);
        return -1;
    }

    //TODO: Verify config writes

    log(LOG_TRACE, sa->self.name, "cfg write 0x%03x count:%i val: 0x%08x", off, count, *(uint32_t*)out);

    /* Copy the configuration space contents from the buffer */
    memcpy( off + (void *) &func->config, out, count );

    func->config.type0.command |= 0x3; //TODO: Proper support for hardwired bits

    /* Signal successful completion */
    return 0;

}

void misa_aunit_bus_target_init( misa_inst *sa, const cfg_section *section ) {
    uint32_t bus_no, dev_no, func_no;

    pci_cfg_handle_type0( &sa->bus_target, section, 0 );
    sa->bus_target.cfg_read = bus_cfg_read;
    sa->bus_target.cfg_write = bus_cfg_write;
    sa->bus_target.mem_read = bus_mem_read;
    sa->bus_target.mem_write = bus_mem_write;

    if ( cfg_find_int32( section, "device_no", &dev_no ) < 0 || dev_no > 31 ) {
        log( LOG_FATAL, section->name, "Missing or invalid PCI device number" );
        exit( EXIT_FAILURE );
    }
    if ( cfg_find_int32( section, "func_no", &func_no ) < 0 || func_no > 7) {
        log( LOG_FATAL, section->name, "Missing or invalid PCI function number" );
        exit( EXIT_FAILURE );
    }
    if ( cfg_find_int32( section, "bus_no", &bus_no ) < 0 || bus_no > 255) {
        log( LOG_FATAL, section->name, "Missing or invalid PCI bus number" );
        exit( EXIT_FAILURE );
    }
    if ( cfg_find_int32( section, "sai", &sa->sai ) < 0 || sa->sai > 255) {
        log( LOG_FATAL, section->name, "Missing or invalid SAI number" );
        exit( EXIT_FAILURE );
    }

    sa->bus_target.bdf    = sa->bdf = PCI_PACK_BDF( bus_no, dev_no, func_no );
    sa->bus_target.device = &sa->self;

    pci_func_register( &sa->bus, &sa->bus_target );

}