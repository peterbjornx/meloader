//
// Created by pbx on 15/08/19.
//

#ifndef MELOADER_FASTSPI_H
#define MELOADER_FASTSPI_H

#include "pci/device.h"
#include "fsb.h"

typedef struct {
    int      csme_readable;
    int      csme_writable;
    uint32_t base;
    uint32_t limit;
} spi_region_t;

typedef struct {
    uint32_t  vscc;
    uint32_t  par_data[0x200];
} spi_component_t;

typedef struct {
    device_instance self;
    pci_simplefunc func;
    spi_component_t spi_components[2];
    spi_region_t spi_regions[12];
    uint32_t     spi_jedec_id[0x40];
    uint32_t     spi_faddr;
    uint32_t     spi_hsflctl;
    uint32_t     spi_ssfsts_ctl;
    uint8_t      spi_buffer[0x40];
    uint32_t     spi_ptinx;
    int          spi_image_file;
    uint32_t     spi_opmenu[2];
    uint32_t     sai;
    uint8_t spi_read_buffer[0x40];
} fastspi_inst;

void spi_openimg( fastspi_inst *spi, const char *path );
int spi_write( fastspi_inst *spi, int addr, const void *buffer, int count );
int spi_read( fastspi_inst *spi, int addr, void *buffer, int count );

#endif //MELOADER_FASTSPI_H
