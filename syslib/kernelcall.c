//
// Created by pbx on 15/04/19.
//

#include <stdint.h>
#include "printf.h"
#include "meloader.h"

int sys_dma_lock_ex( void *par );
int sys_dma_unlock( void *par );
int sys_mg_synclist( void *par );

typedef struct {
    const char *name;
    int size;
    int (*impl)(void *);
} kernelcall_table;

kernelcall_table kctable[] = {
        {
            .name = "sys_snowball_read",
            .size = 16,
            .impl = snowball_read
        },
        {},{},{},{/*4*/},
        {
            .name = "sys_mg_synclist",
            .size = 8,
            .impl = sys_mg_synclist
            },{},{},{/*8*/},
        {},{},{},{/*12*/},{},{},{},{/*16*/},
        {},{},{},{/*20*/},{},{},{},{/*24*/},
        {},
        {
            .name = "sys_dma_lock_ex",
            .size = 52,
            .impl = sys_dma_lock_ex
        },
        {
                .name = "sys_dma_unlock",
                .size = 4,
                .impl = sys_dma_unlock
        }
};

int kernelcall(uint8_t call_id, uint16_t par_sz, void *par) {
    if ( call_id > ( sizeof kctable / sizeof(kernelcall_table))) {
        mel_printf("[libc] syscall( %i, %i, 0x%08x) not impl\n", call_id, par_sz, par);
        return 0;
    }
    if ( par_sz != kctable[call_id].size ){
        mel_printf("[libc] syscall( %i, %i, 0x%08x) wrong size\n", call_id, par_sz, par);
        return 0;
    }
    return kctable[call_id].impl(par);
}