//
// Created by pbx on 20/04/19.
//

#ifndef MELOADER_ATT_REGS_H
#define MELOADER_ATT_REGS_H

#include <stdint.h>

#define ATT_SBADDR_ENDPT(a)    ( a & 0xFFu )
#define ATT_SBADDR_RDOP(a)     ((a >> 8u) & 0xFu)
#define ATT_SBADDR_WROP(a)     ((a >> 16u) & 0xFu)
#define ATT_SBADDR_BAR(a)      ((a >> 24u) & 0x7u)
#define ATT_SBADDH_FUNC(a)     ( a & 0xFFu )
#define ATT_SBADDH_RS(a)       ( (a >> 8u) & 0x3u )

typedef struct __attribute__((packed)) {
    uint32_t INT_BA;
    uint32_t INT_SIZE;
    uint32_t EXT_BA_LO;
    uint32_t EXT_BA_HI;
    uint32_t CONTROL;
    uint32_t reserved[ 3 ];
} att_window;

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

typedef struct {
    att_window    WIN[128];
    att_sb_window SB_WIN[128];
} att_regs;

#endif //MELOADER_REGS_H
