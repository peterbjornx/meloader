//
// Created by pbx on 20/04/19.
//

#include "log.h"
#include "sideband.h"

int sbdev_count = 0;
sideband_dev *sbdev_list[64];

void sb_register( sideband_dev * periph ) {
    sbdev_list[sbdev_count++] = periph;
}

int sb_read( int endp, int bar, int address, void *buffer, int count, int sai ){
    int i =0;
    for ( i = 0; i < sbdev_count; i++ )
        if ( sbdev_list[i]->endpoint == endp)
            return sbdev_list[i]->read( bar, address, buffer, count, sai );
    log( LOG_ERROR, "sideband", "Read to unknown endpoint %02X", endp );
    return -2;
}

int sb_write( int endp, int bar, int address, const void *buffer, int count, int sai ){
    int i =0;
    for ( i = 0; i < sbdev_count; i++ )
        if ( sbdev_list[i]->endpoint == endp)
            return sbdev_list[i]->write( bar, address, buffer, count, sai );
    log( LOG_ERROR, "sideband", "Write to unknown endpoint %02X", endp );
    return -2;
}