//
// Created by pbx on 20/04/19.
//

#ifndef MELOADER_SIDEBAND_H
#define MELOADER_SIDEBAND_H

#include <stdint.h>

typedef struct  {
    uint8_t endpoint;
    int (*write)( int bar, int op, int offset, const void *buffer, int count, int sai );
    int (*read )( int bar, int op, int offset, void *buffer, int count, int sai );

} sideband_dev;

void sb_register( sideband_dev *dev );
int sb_read( int endp, int op, int bar, int address, void *buffer, int count, int sai );
int sb_write( int endp, int op, int bar, int address, const void *buffer, int count, int sai );

#endif //MELOADER_SIDEBAND_H
