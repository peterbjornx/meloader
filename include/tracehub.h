//
// Created by pbx on 15/04/19.
//

#ifndef MELOADER_TRACEHUB_H
#define MELOADER_TRACEHUB_H

#define TRACEHUB_MTB_BASE    (0xF7400000)
#define TRACEHUB_MTB_SIZE    (0x00100000)
#define TRACEHUB_FTMR_BASE   (0xF7000000)
#define TRACEHUB_FTMR_SIZE   (0x00400000)

#define TRACEHUB_GTH_OFFSET     (0x0)
#define TRACEHUB_TSCU_OFFSET    (0x2000)
#define TRACEHUB_CTS_OFFSET     (0x3000)
#define TRACEHUB_STH_OFFSET     (0x4000)
#define TRACEHUB_SoCHAP_OFFSET  (0x5000)
#define TRACEHUB_ODLA_OFFSET    (0x6000)
#define TRACEHUB_VISE_OFFSET    (0x7000)
#define TRACEHUB_VISC_OFFSET    (0x20000)

void tracehub_gth_read(int addr, void *buffer, int count);
void tracehub_gth_write(int addr, const void *buffer, int count);
int tracehub_ftmr_read( int addr, void *buffer, int count );
int tracehub_ftmr_write( int addr, const void *buffer, int count );

#endif //MELOADER_TRACEHUB_H
