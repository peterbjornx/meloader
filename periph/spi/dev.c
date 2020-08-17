//
// Created by pbx on 10/08/19.
//
#include <string.h>
#include <stdlib.h>
#include <fastspi.h>
#include "pci/bus.h"
#include "pci/device.h"
#include "devreg.h"
#include "fastspi.h"
#include "log.h"
#include <stdio.h>

static int fastspi_bar_read(pci_func *func, int bar, uint64_t addr, void *buffer, int count) {
    fastspi_inst *t = func->device->impl;
    if ( bar == 0 ) {
        spi_read(t, addr, buffer, count);
        return t->sai;
    } else if ( bar == 1 ) {
        spi_direct_read(t, addr, buffer, count);
        return t->sai;
    } else if ( bar == 2 ) {
        spi_huffmann_read(t, addr, buffer, count);
        return t->sai;
    }
    log(LOG_ERROR, t->self.name, "Read to unimplemented bar");
    return -1;
}

static int fastspi_bar_write(pci_func *func, int bar, uint64_t addr, const void *buffer, int count) {
    fastspi_inst *t = func->device->impl;
    if ( bar == 0 ) {
        spi_write(t, addr, buffer, count);
        return t->sai;
    } else
        log(LOG_ERROR, t->self.name, "Write to unimplemented bar");
    return -1;
}

void spi_config( fastspi_inst *spi, const cfg_section *section ) {
    char name[40];
    int i;
    const char *path;
    path = cfg_find_string( section, "rom_image" );
    logassert( path != NULL, spi->self.name, "No ROM image specified" );
    spi_openimg( spi, path );
    for ( i = 0; i < 12; i++ ){
        snprintf(name, 40, "region_%i_base", i);
        cfg_find_int32( section, name, &spi->spi_regions[i].base );
        snprintf(name, 40, "region_%i_limit", i);
        cfg_find_int32( section, name, &spi->spi_regions[i].limit );
        snprintf(name, 40, "region_%i_csme_readable", i);
        cfg_find_int32( section, name, &spi->spi_regions[i].csme_readable );
        snprintf(name, 40, "region_%i_csme_writable", i);
        cfg_find_int32( section, name, &spi->spi_regions[i].csme_writable );
    }
    for ( i = 0; i < 0x40; i++ ){
        snprintf(name, 40, "jedec_%02X", i);
        cfg_find_int32( section, name, &spi->spi_jedec_id[i] );
    }
    for ( i = 0; i < 2; i++ ){
        snprintf(name, 40, "comp_%i_vscc", i);
        cfg_find_int32( section, name, &spi->spi_components[i].vscc );
        snprintf(name, 40, "comp_%i_par_data_0", i);
        cfg_find_int32( section, name, &spi->spi_components[i].par_data[0] );
    }
    spi->spi_hsflctl |= 0x4000;
    cfg_find_int32( section, "sai", &spi->sai );

}

static device_instance * fastspi_spawn(const cfg_file *file, const cfg_section *section) {
    uint32_t fp = 0;
    fastspi_inst *i = malloc( sizeof(fastspi_inst) );

    if ( !i ) {
        log( LOG_FATAL, section->name, "Could not allocate Fast SPI instance structure" );
        exit(EXIT_FAILURE);
    }
    memset( i, 0, sizeof(fastspi_inst) );

    i->self.impl = i;
    i->self.name = section->name;
    i->func.bar_size[0] = 0x00001000;
    i->func.bar_size[1] = 0x01000000;
    i->func.bar_size[2] = 0x01000000;
    i->func.max_bar = 3;
    i->func.func.device = &i->self;
    i->func.flags = 0;
    i->func.bar_read = fastspi_bar_read;
    i->func.bar_write = fastspi_bar_write;

    spi_config( i, section );

    pci_simple_init( &i->func, section );

    device_register( &i->self );

    return &i->self;
}

device_type fastspi_type = {
        .name = "fastspi",
        .spawn = fastspi_spawn
};

static __attribute__((constructor)) void register_fastspi() {
    device_type_register( &fastspi_type );
}