//
// Created by pbx on 20/04/19.
//

#ifndef MELOADER_SIDEBAND_H
#define MELOADER_SIDEBAND_H

#include <stdint.h>

typedef struct  {
    uint8_t endpoint;
    int (*write)( int bar, int offset, const void *buffer, int count );
    int (*read )( int bar, int offset, void *buffer, int count );

} sideband_dev;

void sb_register( sideband_dev *dev );
void sb_read( int endp, int bar, int address, void *buffer, int count );
void sb_write( int endp, int bar, int address, const void *buffer, int count );

#endif //MELOADER_SIDEBAND_H
