//
// Created by pbx on 15/04/19.
//

#ifndef MELOADER_TRACEHUB_H
#define MELOADER_TRACEHUB_H

#include "pci/device.h"

#define TRACEHUB_MTB_SIZE    (0x00100000)
#define TRACEHUB_FTMR_SIZE   (0x00400000)

#define TRACEHUB_GTH_OFFSET     (0x0)
#define TRACEHUB_TSCU_OFFSET    (0x2000)
#define TRACEHUB_CTS_OFFSET     (0x3000)
#define TRACEHUB_STH_OFFSET     (0x4000)
#define TRACEHUB_SoCHAP_OFFSET  (0x5000)
#define TRACEHUB_ODLA_OFFSET    (0x6000)
#define TRACEHUB_VISE_OFFSET    (0x7000)
#define TRACEHUB_VISC_OFFSET    (0x20000)

typedef struct {
    device_instance self;
    pci_simplefunc func;
    uint32_t gth_scrpd0;
    uint32_t gth_swdest[32];
} thub_inst;
void tracehub_fake_probe( thub_inst *t );
void tracehub_gth_read( thub_inst *t, uint32_t addr, void *buffer, int count);
void tracehub_gth_write( thub_inst *t, uint32_t addr, const void *buffer, int count);
int tracehub_ftmr_read(  thub_inst *t, uint32_t addr, void *buffer, int count );
int tracehub_ftmr_write(  thub_inst *t, uint32_t addr, const void *buffer, int count );

#endif //MELOADER_TRACEHUB_H
