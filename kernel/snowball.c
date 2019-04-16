//
// Created by pbx on 16/04/19.
//
#include <stdint.h>
#include <string.h>
#include <printf.h>

typedef struct __attribute__((packed)) {
    char     name[12];
    uint8_t  unk0;
    uint8_t  flags;
    uint16_t size;
    uint16_t unk1;
} romhandoff_block;

typedef struct __attribute__((packed)) {
    uint32_t size;
    void *   buffer;
    uint32_t size2;
    uint32_t actualsize;
} handoff_read_pars;

uint8_t snowball_buffer[0x1FE000];
int     snowball_wrptr = 0;
int     snowball_rdptr = 0;

void snowball_add(const char *name, int unk0, int flags, int size, int unk1,
                  void *data) {
    int total_size = size + sizeof(romhandoff_block);
    romhandoff_block *block;
    if ( total_size + snowball_wrptr >= sizeof snowball_buffer )
        return;
    block = (romhandoff_block *) (snowball_buffer + snowball_wrptr);
    strncpy(block->name, name, 12);
    block->unk0 = (uint8_t) unk0;
    block->flags = (uint8_t) (flags | 0x1);
    block->unk1 = (uint16_t) unk1;
    block->size = (uint16_t) total_size;
    memcpy(&block[1], data, (size_t) size);
    snowball_wrptr += total_size;
}

int snowball_read(handoff_read_pars *par) {
    int turnsize = par->size;
    int avail = snowball_wrptr - snowball_rdptr;
    mel_printf("[krnl] sys_snowball_read( %i, %p, %i )\n",
            par->size,par->buffer,par->size2);
    par->actualsize = 0;
    if ( par->size != par->size2 )
        return 0;
    if ( !avail )
        return 0;
    if ( avail < turnsize )
        turnsize = avail;
    par->actualsize = (uint32_t) turnsize;
    memcpy(par->buffer, snowball_buffer + snowball_rdptr, (size_t) turnsize);
    snowball_rdptr += turnsize;
    return 0;
}