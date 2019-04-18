//
// Created by pbx on 15/04/19.
//
#include "meloader.h"
#include "printf.h"

#define SKS_BASE (0xF510F000);
#define SKS_SIZE (0x484)

#define SKS_REG_SLOT        (0x400)
#define SKS_REG_COMMAND     (0x404)
#define SKS_REG_STATUS      (0x408)
#define SKS_REG_DATA(o)     (0x40C+o)
#define SKS_REG_ATR1(i)     (0x42C + 4*i)

#define SKS_STS_BUSY        (1<<0)
#define SKS_STS_ERRR        (1<<1)

#define SKS_CMD_ALWAYS_SET  (1<<31)
#define SKS_CMD_PRODUCE     (1<<18)
#define SKS_CMD_SOURCE_MASK (3<<16)
#define SKS_CMD_SOURCE_CPU  (1<<16)
#define SKS_CMD_SOURCE_AES  (2<<16)
#define SKS_CMD_SOURCE_HASH (3<<16)
#define SKS_CMD_TARGET_MASK (3<<20)
#define SKS_CMD_TARGET_AES  (1<<20)
#define SKS_CMD_TARGET_HASH (2<<20)

#define SKS_ATR1_KEY_VALID  (1<<0)
#define SKS_ATR1_UNK8       (1<<8)

#define SKS_SLOT_WRAPKEY    (21)

typedef struct {
    uint32_t key_atr1;
    void    *key_data;
} sks_key;

uint8_t sks_keystore[32*11+16*11];
sks_key sks_keys[22];
uint32_t sks_slot    = 0;
uint32_t sks_command = 0;
uint32_t sks_status  = 0;

int sks_read(int addr, void *buffer, int count ) {
    int i = 0;
    uint32_t *buf = buffer;
    addr -= SKS_BASE;
    if ( addr < 0 || addr >= SKS_SIZE )
        return 0;
    if ( addr == SKS_REG_SLOT ) {
        *buf = sks_slot;
        mel_printf("[sks ] read  sks_slot     = 0x%08x\n", *buf);
    } else if ( addr == SKS_REG_COMMAND ) {
        mel_printf("[sks ] read  sks_command  = 0x%08x\n", *buf);
        *buf = sks_command;
    } else if ( addr == SKS_REG_STATUS ) {
        *buf = sks_status;
        mel_printf("[sks ] read  sks_status   = 0x%08x\n", *buf);
    } else if ( addr >= SKS_REG_DATA(0) && addr < SKS_REG_DATA(32) ) {
        i = addr - SKS_REG_DATA(0);
        *buf = sks_slot;
    } else if ( addr >= SKS_REG_ATR1(0) && addr < SKS_REG_ATR1(22) ) {
        i = (addr - SKS_REG_ATR1(0)) / 4;
        mel_printf("[sks ] read slot[%i].atr1 = 0x%08x\n", i, *buf);
        *buf = sks_keys[i].key_atr1;
    } else
        mel_printf("[sks ] read  0x%03x count:%i\n", addr, count);
    return 1;
}

int sks_write( int addr, const void *buffer, int count ) {
    const uint32_t *buf = buffer;
    int i;
    addr -= SKS_BASE;
    if ( addr < 0 || addr >= SKS_SIZE )
        return 0;
    if ( addr == SKS_REG_SLOT ) {
        sks_slot = *buf;
        mel_printf("[sks ] write sks_slot     = 0x%08x\n", *buf);
    } else if ( addr == SKS_REG_COMMAND ) {
        sks_command = *buf;
        mel_printf("[sks ] write sks_command  = 0x%08x\n", *buf);
    } else if ( addr == SKS_REG_STATUS ) {
        sks_status = *buf;
        mel_printf("[sks ] write sks_status   = 0x%08x\n", *buf);
    } else if ( addr >= SKS_REG_DATA(0) && addr < SKS_REG_DATA(32) ) {
        i = addr - SKS_REG_DATA(0);
        sks_slot = *buf;
        mel_printf("[sks ] write data[%i / 4] = 0x%08x\n", i, *buf);
    } else if ( addr >= SKS_REG_ATR1(0) && addr < SKS_REG_ATR1(22) ) {
        i = (addr - SKS_REG_ATR1(0)) / 4;
        sks_keys[i].key_atr1 = *buf;
        mel_printf("[sks ] write slot[%i].atr1 = 0x%08x\n", i, *buf);
    } else
        mel_printf("[sks ] write unknown 0x%03x count:%i val: 0x%08x\n", addr, count, *buf);
    return 1;
}


mmio_periph sks_mmio = {
        .write = sks_write,
        .read = sks_read
};

void sks_init(){
    int i;
    memset( sks_keystore, 0, sizeof sks_keystore );
    for ( i = 0; i < 11; i++ ) {
        sks_keys[i].key_data = sks_keystore + 16 * i;
        sks_keys[i].key_atr1 = 0;
    }
    for ( i = 11; i < 22; i++ ) {
        sks_keys[i].key_data = sks_keystore + 16 * 11 + 32 * i;
        sks_keys[i].key_atr1 = 0;
    }
}

void sks_install_fakekeys() {
    sks_keys[21].key_atr1 |= SKS_ATR1_KEY_VALID;
}

void sks_install() {
    sks_init();
    sks_install_fakekeys();
    krnl_periph_reg( &sks_mmio );
}