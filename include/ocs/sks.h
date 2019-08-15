//
// Created by pbx on 14/08/19.
//

#ifndef MELOADER_SKS_H
#define MELOADER_SKS_H

#include <stdint.h>

#define SKS_REG_SLOT        (0x400)
#define SKS_REG_COMMAND     (0x404)
#define SKS_REG_STATUS      (0x408)
#define SKS_REG_DATA(o)     (0x40C + o)
#define SKS_REG_ATR1(i)     (0x42C + 4*i)

#define SKS_STS_BUSY        (1u<<0u)
#define SKS_STS_ERRR        (1u<<1u)

#define SKS_CMD_ALWAYS_SET  (1u<<31u)
#define SKS_CMD_PRODUCE     (1u<<18u)
#define SKS_CMD_SOURCE_MASK (3u<<16u)
#define SKS_CMD_SOURCE_CPU  (1u<<16u)
#define SKS_CMD_SOURCE_AES  (2u<<16u)
#define SKS_CMD_SOURCE_HASH (3u<<16u)
#define SKS_CMD_TARGET_MASK (3u<<20u)
#define SKS_CMD_TARGET_AES  (1u<<20u)
#define SKS_CMD_TARGET_HASH (2u<<20u)

#define SKS_ATR1_KEY_VALID  (1u<<0u)
#define SKS_ATR1_UNK8       (1u<<8u)

#define SKS_SLOT_WRAPKEY    (21)


typedef struct {
    uint32_t key_atr1;
    void    *key_data;
} sks_key;

typedef struct {
    const char *name;
    uint8_t sks_keystore[32*11+16*11];
    sks_key sks_keys[22];
    uint32_t sks_slot;
    uint32_t sks_command;
    uint32_t sks_status;
    uint8_t  sks_databuf[32];
    void *hash;
    void *aes;
    int (*aes_get_result)( void *aes, void *data, size_t count);
    int (*hash_get_result)( void *hash, void *data, size_t count);
    void (*aes_load_key)( void *aes, void *data, size_t count );
    void (*hash_load_key)( void *hash, void *data, size_t count );
} sks_inst;

void sks_init( device_instance *parent, sks_inst *sks );
int sks_read( sks_inst *sks, uint32_t addr, void *buffer, int count );
int sks_write( sks_inst *sks, uint32_t addr, const void *buffer, int count );
void sks_do_operation();

#endif //MELOADER_SKS_H
