//
// Created by pbx on 20/04/19.
//

#include "log.h"
#include "sideband.h"

sideband_dev *sbdev_list;

void sb_register( sideband_dev * periph ) {
    periph->next = sbdev_list;
    sbdev_list = periph;
}

int sb_read( int endp, int op, int bar, int address, void *buffer, int count, int sai ){
    sideband_dev *dev;
    for ( dev = sbdev_list; dev; dev = dev->next )
        if ( dev->endpoint == endp)
            return dev->read( dev, bar, op, address, buffer, count, sai );
    log( LOG_ERROR, "sideband", "Read to unknown endpoint %02X", endp );
    return -2;
}

int sb_write( int endp, int op, int bar, int address, const void *buffer, int count, int sai ){
    sideband_dev *dev;
    for ( dev = sbdev_list; dev; dev = dev->next )
        if ( dev->endpoint == endp)
            return dev->write( dev, bar, op, address, buffer, count, sai );
    log( LOG_ERROR, "sideband", "Write to unknown endpoint %02X", endp );
    return -2;
}