//
// Created by pbx on 19/07/19.
//

#ifndef MELOADER_PCI_DEVICE_H
#define MELOADER_PCI_DEVICE_H

#include "pci/bus.h"
#include "pci/device.h"

#define PCI_SIMPLEFUNC_64BIT    (1u<<0u)

typedef int  (*pci_bar_read)      ( pci_func *func, int bar, uint64_t offset,       void *buf, int count );
typedef int  (*pci_bar_write)     ( pci_func *func, int bar, uint64_t offset, const void *buf, int count );
typedef void (*pci_cfg_preread)   ( pci_func *func, int offset );
typedef void (*pci_cfg_postwrite) ( pci_func *func, int offset );
typedef struct pci_simplefunc_s pci_simplefunc;

struct pci_simplefunc_s {
    pci_func            func;
    uint32_t            flags;
    int                 max_bar;
    int                 bar_size[6];
    pci_bar_read        bar_read;
    pci_bar_write       bar_write;
    pci_cfg_preread     cfg_read;
    pci_cfg_postwrite   cfg_write;
};

void pci_simple_init( pci_simplefunc *func, const cfg_section *section );
void pci_cfg_handle_type0( pci_func *func, const cfg_section *section, int bit64 );
void pci_cfg_handle_device( pci_func *func, const cfg_section *section );

#endif //MELOADER_DEVICE_H
