//
// Created by pbx on 03/04/19.
//

#ifndef ROMLIB_H
#define ROMLIB_H

long long tstamp_read( void );
void write_seg_32(int seg, int offset, int value);
void write_seg_16(int seg, int offset, short value);
void write_seg_8 (int seg, int offset, char value);
int   read_seg_32(int seg, int offset);
short read_seg_16(int seg, int offset);
char  read_seg_8 (int seg, int offset);
void write_seg   (int seg, int offset, const void *buffer, int count);
void  read_seg   (void *buffer, int seg, int offset, int count);

#endif //ROMLIB_H
