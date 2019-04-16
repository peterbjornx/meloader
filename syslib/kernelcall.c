//
// Created by pbx on 15/04/19.
//

#include <stdint.h>
#include "printf.h"

int kernelcall(uint8_t call_id, uint16_t par_sz, void *par) {
    mel_printf("[libc] syscall( %i, %i, 0x%08x)\n", call_id, par_sz, par);
    return 0;
}