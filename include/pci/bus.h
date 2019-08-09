//
// Created by pbx on 20/06/19.
//

#ifndef MELOADER_PCI_BUS_H
#define MELOADER_PCI_BUS_H

#include "pci/cfgspace.h"
#include "devreg.h"

#define PCI_CONFIG_SIZE     (0x1000)
#define PCI_ADDR_TYPE0      (0x00000000u)
#define PCI_ADDR_TYPE1      (0x10000000u)
#define PCI_ADDR_TYPE0_MASK (0x00007FFFu)
#define PCI_ADDR_TYPE1_MASK (0x0FFFFFFFu)
#define PCI_ADDR_BDF_MASK   (0x0FFFF000u)
#define PCI_ADDR_TYPE(A)    ((A)&0x10000000u)
#define PCI_ADDR_BUS(A)     (((A)>>20u)&0xFFu)
#define PCI_ADDR_DEV(A)     (((A)>>15u)&0x1Fu)
#define PCI_ADDR_FUNC(A)    (((A)>>12u)&0x7u)
#define PCI_ADDR_REG(A)     ((A)&0xFFFu)
#define PCI_PACK_BDF( B, D, F ) ( (((B) << 8u) | ((D) << 3u) | (F)) << 12u )
#define PCI_BDF_DF_MASK     (0x000FF000u)
#define PCI_PACK_ADDR( B, D, F, O ) ( PCI_PACK_BDF( B, D, F ) | (O) )

typedef struct pci_bus_s        pci_bus;
typedef struct pci_func_s       pci_func;

struct pci_bus_s {
    pci_bus         *next;
    const char      *name;
    pci_func        *first_func;
    unsigned int     bus_num;
};

struct pci_func_s {
    pci_bus         *bus;
    pci_func        *next;
    uint32_t         bdf;
    device_instance *device;
    pci_cfgspace     config;
    int              (*cfg_read) ( pci_func *func, uint64_t addr,       void *out, int count );
    int              (*cfg_write)( pci_func *func, uint64_t addr, const void *out, int count );
    int              (*mem_read) ( pci_func *func, uint64_t addr,       void *out, int count, int sai, int lat );
    int              (*mem_write)( pci_func *func, uint64_t addr, const void *out, int count, int sai, int lat );
};

void      pci_bus_register    ( pci_bus *bus );
pci_bus  *pci_bus_find        ( const char *name );
void      pci_func_register   ( pci_bus *bus, pci_func *func );
pci_func *pci_func_find       ( pci_bus *bus, uint32_t bdf );
void      pci_set_bus_num     ( pci_bus *bus, unsigned int bus_num );
int       pci_bus_config_read ( pci_bus *bus, uint64_t addr,       void *out, int count );
int       pci_bus_config_write( pci_bus *bus, uint64_t addr, const void *out, int count );
int       pci_bus_mem_read    ( pci_bus *bus, uint64_t addr,       void *out, int count, int sai, int max_lat );
int       pci_bus_mem_write   ( pci_bus *bus, uint64_t addr, const void *out, int count, int sai, int max_lat );
int       pci_bus_io_read     ( pci_bus *bus, uint64_t addr,       void *out, int count, int sai, int max_lat );
int       pci_bus_io_write    ( pci_bus *bus, uint64_t addr, const void *out, int count, int sai, int max_lat );
#endif //MELOADER_PCI_BUS_H
