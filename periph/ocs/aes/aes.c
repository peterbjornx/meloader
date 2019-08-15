//
// Created by pbx on 15/04/19.
//
#include "user/meloader.h"
#include "printf.h"

#define AES_BASE (0xF5108000);
#define AES_SIZE (0x1000)

int aes_read(int addr, void *buffer, int count ) {
    int i = 0;
    uint32_t *buf = buffer;
    addr -= AES_BASE;
    if ( addr < 0 || addr >= AES_SIZE )
        return 0;
    if ( addr == 0x05c)
        *buf = 2;
    mel_printf("[aes ] read  unknown 0x%03x count:%i val: 0x%08x\n", addr, count, *buf);
    return 1;
}

int aes_write( int addr, const void *buffer, int count ) {
    const uint32_t *buf = buffer;
    int i;
    addr -= AES_BASE;
    if ( addr < 0 || addr >= AES_SIZE )
        return 0;
    mel_printf("[aes ] write unknown 0x%03x count:%i val: 0x%08x\n", addr, count, *buf);
    return 1;
}

int aes_get_result( void *data, size_t count ) {
    return 0;
}

void aes_load_key( void *data, size_t count ) {
}

mmio_periph aes_mmio = {
        .write = aes_write,
        .read = aes_read
};

void aes_init(){
}

void aes_install() {
    aes_init();
    krnl_periph_reg( &aes_mmio );
}