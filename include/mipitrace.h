//
// Created by pbx on 15/04/19.
//

#ifndef MELOADER_MIPITRACE_H
#define MELOADER_MIPITRACE_H

#include <stdint.h>

#define MT_Dn (0)
#define MT_DnM (1)
#define MT_DnTS (2)
#define MT_DnMTS (3)
#define MT_USER (4)
#define MT_USER_TS (5)
#define MT_FLAG (6)
#define MT_FLAG_TS (7)
#define MT_MERR (8)

extern const char *mipi_tmsg_types[];

struct th_msg {
    int type;
    int size;
    union {
        uint8_t  d8;
        uint16_t d16;
        uint32_t d32;
        uint64_t d64;
    };
};

void trace_msg(int master, int chan, struct th_msg msg);

#endif //MELOADER_MIPITRACE_H
