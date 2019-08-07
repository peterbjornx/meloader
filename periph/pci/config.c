//
// Created by pbx on 19/07/19.
//

#include <stdlib.h>
#include "log.h"
#include "pci/bus.h"
#include "cfg_file.h"

void pci_cfg_handle_type0( pci_func *func, const cfg_section *section, int bit64 ) {
    cfg_find_int16(section, "device_id", &func->config.type0.did );
    cfg_find_int16(section, "vendor_id", &func->config.type0.vid );
    cfg_find_int16(section, "command"  , &func->config.type0.command );
    cfg_find_int16(section, "status",    &func->config.type0.status );
    if ( bit64 ) {
        cfg_find_int32(section, "bar0", &func->config.type0.bar[0] );
        cfg_find_int32(section, "bar1", &func->config.type0.bar[1] );
        cfg_find_int32(section, "bar2", &func->config.type0.bar[2] );
        cfg_find_int32(section, "bar3", &func->config.type0.bar[3] );
        cfg_find_int32(section, "bar4", &func->config.type0.bar[4] );
        cfg_find_int32(section, "bar5", &func->config.type0.bar[5] );
    } else {
        cfg_find_int64(section, "bar0", &func->config.type0.bar64[0] );
        cfg_find_int64(section, "bar1", &func->config.type0.bar64[1] );
        cfg_find_int64(section, "bar2", &func->config.type0.bar64[2] );
    }
}

void pci_cfg_handle_device( pci_func *func, const cfg_section *section ) {
    uint32_t dev_no, func_no;
    const char *bus_name;
    pci_bus *bus;

    if ( cfg_find_int32( section, "device_no", &dev_no ) < 0 || dev_no > 31 ) {
        log( LOG_FATAL, section->name, "Missing or invalid PCI device number" );
        exit( EXIT_FAILURE );
    }
    if ( cfg_find_int32( section, "func_no", &func_no ) < 0 || func_no > 7) {
        log( LOG_FATAL, section->name, "Missing or invalid PCI function number" );
        exit( EXIT_FAILURE );
    }

    func->bdf = PCI_PACK_BDF( 0u, dev_no, func_no );

    bus_name = cfg_find_string( section, "bus" );
    if ( !bus_name ) {
        log( LOG_FATAL, section->name, "Missing PCI bus name" );
        exit( EXIT_FAILURE );
    }

    bus = pci_bus_find( bus_name );
    if ( !bus ) {
        log( LOG_FATAL, section->name, "Could not find PCI bus %s", bus_name );
        exit( EXIT_FAILURE );
    }

    pci_func_register( bus, func );
}