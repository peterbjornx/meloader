//
// Created by pbx on 03/04/19.
//

#include <stdio.h>
#include "meloader.h"

long long tstamp_read(void ) {
    return 0x7EADCAFEFEED1337LL;
}

void write_seg_32(int seg, int offset, int value) {
    write_str("[trace] write_seg_32( 0x");
    write_hex(4, seg);
    write_str(", 0x");
    write_hex(8, offset);
    write_str(", 0x");
    write_hex(8, value);
    write_str(")\n");
    krnl_write_seg( seg, offset, &value, sizeof value );

}
void write_seg_16(int seg, int offset, short value) {
    write_str("[trace] write_seg_16( 0x");
    write_hex(4, seg);
    write_str(", 0x");
    write_hex(8, offset);
    write_str(", 0x");
    write_hex(4, value);
    write_str(")");
    krnl_write_seg( seg, offset, &value, sizeof value );

}
void write_seg_8 (int seg, int offset, char value) {
    write_str("[trace] write_seg_8( 0x");
    write_hex(4, seg);
    write_str(", 0x");
    write_hex(8, offset);
    write_str(", 0x");
    write_hex(2, value);
    write_str(")\n");
    krnl_write_seg( seg, offset, &value, sizeof value );

}

int   read_seg_32(int seg, int offset) {
    int value=0;
    if (seg != 0x3B) {
        write_str("[trace] read_seg_32( 0x");
        write_hex(4, seg);
        write_str(", 0x");
        write_hex(8, offset);
        write_str(")\n");
    } else
        value = 0xDEADBEEF;
    krnl_read_seg( seg, offset, &value, sizeof value );
    return value;
}
short  read_seg_16(int seg, int offset) {
    short value=0;
    write_str("[trace] read_seg_16( 0x");
    write_hex(4, seg);
    write_str(", 0x");
    write_hex(8, offset);
    write_str(")\n");
    krnl_read_seg( seg, offset, &value, sizeof value );
    return value;

}
char  read_seg_8 (int seg, int offset) {
    char value=0;
    write_str("[trace] read_seg_8( 0x");
    write_hex(4, seg);
    write_str(", 0x");
    write_hex(8, offset);
    write_str(")\n");
    krnl_read_seg( seg, offset, &value, sizeof value );
    return value;
}
void write_seg   (int seg, int offset, const void *buffer, int count) {
    write_str("[trace] write_seg( 0x");
    write_hex(4, seg);
    write_str(", 0x");
    write_hex(8, offset);
    write_str(", 0x");
    write_hex(8, buffer);
    write_str(", 0x");
    write_hex(8, count);
    write_str(")\n");
    krnl_write_seg(seg, offset, buffer, (size_t) count);
}
void read_seg    (void *buffer, int seg, int offset, int count) {
    write_str("[trace] read_seg( 0x");
    write_hex(8, buffer);
    write_str(", 0x");
    write_hex(4, seg);
    write_str(", 0x");
    write_hex(8, offset);
    write_str(", 0x");
    write_hex(8, count);
    write_str(")\n");
    krnl_read_seg(seg, offset, buffer, (size_t) count);

}
