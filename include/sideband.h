//
// Created by pbx on 20/04/19.
//

#ifndef MELOADER_SIDEBAND_H
#define MELOADER_SIDEBAND_H

#include <stdint.h>
#include "devreg.h"

typedef struct sideband_dev_s sideband_dev;

struct sideband_dev_s {
    sideband_dev *next;
    device_instance *device;
    uint8_t endpoint;
    int (*write)( sideband_dev *dev, int bar, int op, int offset, const void *buffer, int count, int sai );
    int (*read )( sideband_dev *dev, int bar, int op, int offset, void *buffer, int count, int sai );
};

void sb_register( sideband_dev *dev );
int sb_read( int endp, int op, int bar, int address, void *buffer, int count, int sai );
int sb_write( int endp, int op, int bar, int address, const void *buffer, int count, int sai );

#endif //MELOADER_SIDEBAND_H
