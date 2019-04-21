//
// Created by pbx on 20/04/19.
//

#ifndef MELOADER_ATT_H
#define MELOADER_ATT_H

#include <stdint.h>

typedef struct {
    uint32_t window_base;
    uint32_t window_size;
    uint32_t window_flags;
    uint32_t reg_C;
    uint32_t reg_10;
    uint32_t reg_14;
    uint32_t sb_address;
    uint32_t reg_1C;
} att_sb_window;

att_sb_window *att_find_sb_win( int address );

void att_install();

#endif //MELOADER_ATT_H
