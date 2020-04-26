//
// Created by pbx on 10/08/19.
//
#include <string.h>
#include <stdlib.h>
#include <pci/device.h>
#include "log.h"
#include "pci/bus.h"
#include "fsb.h"
#include "gasket/prim2gasket/dev.h"

static int p2g_cfg_read ( pci_func *func, uint64_t addr, void *out, int count ){
    p2g_inst *i = func->device->impl;
    addr &= PCI_ADDR_TYPE1_MASK;
    addr |= PCI_ADDR_TYPE1;
    for ( func = i->gasket_bus.first_func; func; func = func->next )
        if ( func->cfg_read && func->cfg_read( func, addr, out, count ) == 0 )
            return 0;
    return -1;
}

static int p2g_cfg_write( pci_func *func, uint64_t addr, const void *out, int count ){
    p2g_inst *i = func->device->impl;
    addr &= PCI_ADDR_TYPE1_MASK;
    addr |= PCI_ADDR_TYPE1;
    for ( func = i->gasket_bus.first_func; func; func = func->next )
        if ( func->cfg_write && func->cfg_write( func, addr, out, count ) == 0 )
            return 0;
    return -1;
}

static int p2g_mem_write( pci_func *func, uint64_t addr, const void *buf, int count, int sai, int lat ) {
    int s;
    p2g_inst *i = func->device->impl;
    if ( (addr & 0xFF000000) == 0xF1000000 ) {
        s = pci_bus_config_write( &i->gasket_bus, addr & 0x00FFFFFFu, buf, count );

        return s;
    } else if ( addr >= 0xE0000000 )
        return pci_bus_mem_write( &i->gasket_bus, addr, buf, count, sai, 16 - lat );
    else
        return -1;
}

static int p2g_mem_read( pci_func *func, uint64_t addr,       void *buf, int count, int sai, int lat ) {
    int s;
    p2g_inst *i = func->device->impl;
    if ( (addr & 0xFF000000) == 0xF1000000 ) {
        s = pci_bus_config_read( &i->gasket_bus, addr & 0x00FFFFFFu, buf, count );

        return s;
    } else if ( addr >= 0xE0000000 )
        return pci_bus_mem_read( &i->gasket_bus, addr, buf, count, sai, 16 - lat );
    else
        return -1;
}

static int p2g_ret_mem_write( pci_func *func, uint64_t addr, const void *buf, int count, int sai, int lat ) {
    p2g_inst *i = func->device->impl;
    if ( addr < 0xE0000000 )
        return pci_bus_mem_write( i->func.bus, addr, buf, count, sai, 16 - lat );
    return -1;
}

static int p2g_ret_mem_read ( pci_func *func, uint64_t addr,       void *buf, int count, int sai, int lat ) {
    p2g_inst *i = func->device->impl;
    if ( addr < 0xE0000000 )
        return pci_bus_mem_read( i->func.bus, addr, buf, count, sai, 16 - lat );
    return -1;
}

static device_instance * p2g_spawn(const cfg_file *file, const cfg_section *section) {
    pci_bus *bus;
    const char *name;
    p2g_inst *i = malloc( sizeof(p2g_inst) );

    if ( !i ) {
        log( LOG_FATAL, section->name, "Could not allocate prim2gasket instance structure" );
        exit(EXIT_FAILURE);
    }
    memset( i, 0, sizeof(p2g_inst) );

    name = cfg_find_string( section, "prim_bus" );
    logassert( name != NULL, section->name, "No primary bus specified in config");
    bus = pci_bus_find( name );
    logassert( bus != NULL, section->name, "Primary bus %s not found", name);

    name = cfg_find_string( section, "gasket_bus" );
    logassert( name != NULL, section->name, "No gasket bus name specified in config");

    i->gasket_bus.name = name;
    i->gasket_bus.bus_num = 0;
    pci_bus_register(&i->gasket_bus);

    i->self.impl = i;
    i->self.name = section->name;

    i->func.device = &i->self;
    i->func.bdf = PCI_PACK_BDF(0u,0u,1u);
    i->func.cfg_read  = p2g_cfg_read;
    i->func.cfg_write = p2g_cfg_write;
    i->func.mem_read  = p2g_mem_read;
    i->func.mem_write = p2g_mem_write;

    pci_func_register( bus, &i->func );

    i->ret_func.device    = &i->self;
    i->ret_func.mem_read  = p2g_ret_mem_read;
    i->ret_func.mem_write = p2g_ret_mem_write;
    i->ret_func.cfg_read  = NULL;
    i->ret_func.cfg_write = NULL;

    pci_func_register( &i->gasket_bus, &i->ret_func );

    device_register( &i->self );

    return &i->self;
}

device_type p2g_type = {
        .name = "prim2gasket",
        .spawn = p2g_spawn
};

static __attribute__((constructor)) void register_p2g() {
    device_type_register( &p2g_type );
}
