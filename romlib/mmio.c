//
// Created by pbx on 03/04/19.
//

#include <stdio.h>
#include "meloader.h"

long long tstamp_read(void ) {
    return 0x7EADCAFEFEED1337LL;
}

void write_seg_32(int seg, int offset, int value) {
    krnl_write_seg( seg, offset, &value, sizeof value );

}
void write_seg_16(int seg, int offset, short value) {
    krnl_write_seg( seg, offset, &value, sizeof value );

}
void write_seg_8 (int seg, int offset, char value) {
    krnl_write_seg( seg, offset, &value, sizeof value );

}

int   read_seg_32(int seg, int offset) {
    int value=0;
    if (seg == 0x3B)
        value = 0xDEADBEEF;
    krnl_read_seg( seg, offset, &value, sizeof value );
    return value;
}
short  read_seg_16(int seg, int offset) {
    short value=0;
    krnl_read_seg( seg, offset, &value, sizeof value );
    return value;

}
char  read_seg_8 (int seg, int offset) {
    char value=0;
    krnl_read_seg( seg, offset, &value, sizeof value );
    return value;
}
void write_seg   (int seg, int offset, const void *buffer, int count) {
    krnl_write_seg(seg, offset, buffer, (size_t) count);
}
void read_seg    (void *buffer, int seg, int offset, int count) {
    krnl_read_seg(seg, offset, buffer, (size_t) count);

}
