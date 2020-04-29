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

#define SPI_HDLUT_CACHE_SIZE (0x1000)
#define SPI_HDBLK_CACHE_SIZE (0x20)

typedef struct {
    uint32_t sz;
    uint32_t cs;
    uint8_t  arr[32];
} huff_map_t;

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
    uint8_t      spi_read_buffer[0x40];
    uint32_t     spi_hdcompoff;
    uint32_t     cached_hdlut_base;
    uint32_t     cached_hdlut_size;
    uint32_t     cached_hdlut[SPI_HDLUT_CACHE_SIZE / sizeof(uint32_t)];
    uint32_t     cached_hdblk_base;
    uint8_t      cached_hdblk[SPI_HDBLK_CACHE_SIZE];
    uint8_t      page_buf[ 4096 ];


} fastspi_inst;

void spi_openimg( fastspi_inst *spi, const char *path );
int spi_write( fastspi_inst *spi, int addr, const void *buffer, int count );
int spi_read( fastspi_inst *spi, int addr, void *buffer, int count );
int spi_direct_read( fastspi_inst *spi, int addr, void *buffer, int count );
int spi_huffmann_read( fastspi_inst *spi, int addr, void *buffer, int count );
int spi_readimg( fastspi_inst *spi, int start, int count, void *buffer );
int spi_read_csme( fastspi_inst *spi, int start, int count, void *buffer );
#endif //MELOADER_FASTSPI_H
