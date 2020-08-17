//
// Created by pbx on 10/08/19.
//
#include <string.h>
#include <stdlib.h>
#include <pci/device.h>
#include "log.h"
#include "pci/bus.h"
#include "ocs/dev.h"
#include "ocs/sks.h"
#include "ocs/aes.h"
#include "ocs/gp.h"

static int ocs_cfg_read( pci_func *func, uint64_t addr,       void *out, int count )
{
    int off;
    ocs_inst *sa;

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

static int ocs_cfg_write( pci_func *func, uint64_t addr, const void *out, int count )
{
    int off;
    ocs_inst *sa;

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

    /* Copy the configuration space contents from the buffer */
    memcpy( off + (void *) &func->config, out, count );

    /* Signal successful completion */
    return 0;

}

static int ocs_mem_write( pci_func *func, uint64_t addr, const void *buf, int count, int sai, int lat ) {
    int bar, s;
    uint32_t base, off, unit;
    ocs_inst *i = func->device->impl;


    /* Match the address against our BARs */
    for ( bar = 0; bar < 1; bar++ ) {
        base = func->config.type0.bar[bar]   & PCI_BAR_ADDRESS_MASK;
        if ( addr < base)
            continue;
        off = addr - base;
        if ( off >= 0x10000 )
            continue;
        break;
    }

    /* Check if the access hit one of our BARs */
    if ( bar == 1 )
        return -1;

    /* Validate the target addresses */
    if ( off + count > 0x10000 ) {
        log( LOG_ERROR,
             func->device->name,
             "Memory write beyond bounds"
             "( BAR %i, Offset: 0x%03x, Size: 0x%x )",
             bar, off, count);
        return -1;
    }

    /* Check if memory space is enabled */
    if ( ~func->config.type0.command & PCI_COMMAND_MSE ) {
        log( LOG_WARN,
             func->device->name,
             "Memory write while memory space disabled"
             "( BAR %i, Offset: 0x%03x, Size: 0x%x )",
             bar, off, count);
        return -1;
    }

    unit = addr & 0xF000u;
    addr = addr & 0x0FFFu;

    switch ( unit ) {
        case 0x8000:
            s = aes_write( &i->aes, addr, buf, count );
            break;
        case 0xA000:
            s = aes_write( &i->aes2, addr, buf, count );
            break;
        case 0xB000:
            s = hash_write( &i->hash, addr, buf, count );
            break;
        case 0xD000:
            s = gp_write( &i->gp, addr, buf, count );
            break;
        case 0xE000:
            s = rsa_write( &i->rsa, addr, buf, count );
            break;
        case 0xF000:
            s = sks_write( &i->sks, addr, buf, count );
            break;
        default:

            log( LOG_WARN,
                 func->device->name,
                 "Memory write on OCS"
                 "( BAR %i, Offset: 0x%03x, Size: 0x%x )",
                 bar, off, count);

            return 0;
    }

    if (!s)
        return 0;
    return i->sai;
}

static int ocs_mem_read( pci_func *func, uint64_t addr,       void *buf, int count, int sai, int lat ) {
    int bar, s;
    uint32_t base, off, unit;
    ocs_inst *i = func->device->impl;


    /* Match the address against our BARs */
    for ( bar = 0; bar < 1; bar++ ) {
        base = func->config.type0.bar[bar]   & PCI_BAR_ADDRESS_MASK;
        if ( addr < base)
            continue;
        off = addr - base;
        if ( off >= 0x10000 )
            continue;
        break;
    }

    /* Check if the access hit one of our BARs */
    if ( bar == 1 )
        return -1;

    /* Validate the target addresses */
    if ( off + count > 0x10000 ) {
        log( LOG_ERROR,
             func->device->name,
             "Memory read beyond bounds"
             "( BAR %i, Offset: 0x%03x, Size: 0x%x )",
             bar, off, count);
        return -1;
    }

    /* Check if memory space is enabled */
    if ( ~func->config.type0.command & PCI_COMMAND_MSE ) {
        log( LOG_WARN,
             func->device->name,
             "Memory read while memory space disabled"
             "( BAR %i, Offset: 0x%03x, Size: 0x%x )",
             bar, off, count);
        return -1;
    }

    unit = addr & 0xF000u;
    addr = addr & 0x0FFFu;

    switch ( unit ) {
        case 0x8000:
            s = aes_read( &i->aes, addr, buf, count );
            break;
        case 0xA000:
            s = aes_read( &i->aes2, addr, buf, count );
            break;
        case 0xB000:
            s = hash_read( &i->hash, addr, buf, count );
            break;
        case 0xD000:
            s = gp_read( &i->gp, addr, buf, count );
            break;
        case 0xE000:
            s = rsa_read( &i->rsa, addr, buf, count );
            break;
        case 0xF000:
            s = sks_read( &i->sks, addr, buf, count );
            break;
        default:

            log( LOG_WARN,
                 func->device->name,
                 "Memory read on OCS"
                 "( BAR %i, Offset: 0x%03x, Size: 0x%x )",
                 bar, off, count);

            return -2;
    }

    if (!s)
        return -2;
    return i->sai;
}

static int ocs_dma_read( ocs_inst *i, uint32_t addr, void *buffer, size_t count ) {
    return pci_bus_mem_read( i->func.bus, addr, buffer, count, i->sai, 15 );
}

static int ocs_dma_write( ocs_inst *i, uint32_t addr, void *buffer, size_t count ) {
    return pci_bus_mem_write( i->func.bus, addr, buffer, count, i->sai, 15 );
}

static device_instance * ocs_spawn(const cfg_file *file, const cfg_section *section) {
    pci_bus *bus;
    const char *name;
    uint32_t bus_no, dev_no, func_no;
    ocs_inst *i = malloc( sizeof(ocs_inst) );

    if ( !i ) {
        log( LOG_FATAL, section->name, "Could not allocate OCS instance structure" );
        exit(EXIT_FAILURE);
    }
    memset( i, 0, sizeof(ocs_inst) );

    i->self.impl = i;
    i->self.name = section->name;;

    name = cfg_find_string( section, "bus" );
    logassert( name != NULL, section->name, "No primary bus specified in config");
    bus = pci_bus_find( name );
    logassert( bus != NULL, section->name, "Primary bus %s not found", name);

    pci_cfg_handle_type0( &i->func, section, 0 );
    i->func.cfg_read = ocs_cfg_read;
    i->func.cfg_write = ocs_cfg_write;
    i->func.mem_read = ocs_mem_read;
    i->func.mem_write = ocs_mem_write;

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

    if ( cfg_find_int32( section, "sai", &i->sai ) < 0 || i->sai > 255 ) {
        log( LOG_FATAL, section->name, "Missing or invalid SAI number" );
        exit( EXIT_FAILURE );
    }

    i->func.bdf    = i->bdf = PCI_PACK_BDF( bus_no, dev_no, func_no );
    i->func.device = &i->self;

    pci_func_register( bus, &i->func );

    device_register( &i->self );

    sks_init( &i->self, &i->sks );
    hash_init( &i->self, &i->hash );
    aes_init( &i->self, &i->aes, 'a' );
    aes_init( &i->self, &i->aes2, 'b' );
    gp_init( &i->self, &i->gp );
    rsa_init( &i->self, &i->rsa );

    i->sks.hash = &i->hash;
    i->sks.hash_get_result = hash_get_result;
    i->sks.hash_load_key = hash_load_key;
    i->sks.aes_get_result = aes_get_result;
    i->sks.aes_load_key = aes_load_key;
    i->hash.hash_gpdma.bus_impl = i;
    i->hash.hash_gpdma.bus_read = ocs_dma_read;
    i->hash.hash_gpdma.bus_write = ocs_dma_write;
    i->aes.aes_gpdma.bus_impl = i;
    i->aes.aes_gpdma.bus_read = ocs_dma_read;
    i->aes.aes_gpdma.bus_write = ocs_dma_write;
    i->aes2.aes_gpdma.bus_impl = i;
    i->aes2.aes_gpdma.bus_read = ocs_dma_read;
    i->aes2.aes_gpdma.bus_write = ocs_dma_write;
    i->gp.gp_gpdma.bus_impl = i;
    i->gp.gp_gpdma.bus_read = ocs_dma_read;
    i->gp.gp_gpdma.bus_write = ocs_dma_write;

    return &i->self;
}

device_type ocs_type = {
        .name = "ocs",
        .spawn = ocs_spawn
};

static __attribute__((constructor)) void register_ocs() {
    device_type_register( &ocs_type );
}
