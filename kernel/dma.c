//
// Created by pbx on 19/04/19.
//
#include <stdint.h>
#include "printf.h"

typedef struct __attribute__((packed)) {
    uint16_t tid;
    uint8_t  par0a;
    uint8_t  par0b;
    uint32_t grant;//4
    uint32_t par2;//8
    uint32_t address_out;//C
    uint32_t par4;//10
    uint32_t par5;//14
    uint32_t par6;//18
    uint32_t par7;//1C
    uint32_t size;//20
    uint32_t par9;//24
    uint32_t parA;//28
    uint32_t parB;//2C
    uint32_t ref_out;//30
} sys_dma_lock_ex_par;

int sys_dma_lock_ex( sys_dma_lock_ex_par *par ) {
    mel_printf("[krnl] sys_dma_lock_ex()\n");
    par->ref_out = 0x42;//ID
    par->address_out = 0x13;//Address
    par->par7 = 0x37;
    return 0;
}

int sys_dma_unlock( int *par ) {
    mel_printf("[krnl] sys_dma_unlock(0x%08x)\n",*par);
    return 0;
}