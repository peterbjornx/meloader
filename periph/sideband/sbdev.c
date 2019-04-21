//
// Created by pbx on 20/04/19.
//

#include "sideband.h"

int sbdev_count = 0;
sideband_dev *sbdev_list[64];

void sb_register( sideband_dev * periph ) {
    sbdev_list[sbdev_count++] = periph;
}
void sb_read( int endp, int bar, int address, void *buffer, int count ){
    int i =0;
    for ( i = 0; i < sbdev_count; i++ )
        if ( sbdev_list[i]->endpoint == endp)
            sbdev_list[i]->read(bar, address, buffer, count);
}
void sb_write( int endp, int bar, int address, const void *buffer, int count ){
    int i =0;
    for ( i = 0; i < sbdev_count; i++ )
        if ( sbdev_list[i]->endpoint == endp)
            sbdev_list[i]->write(bar, address, buffer, count);

}