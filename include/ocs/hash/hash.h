//
// Created by pbx on 14/08/19.
//

#ifndef MELOADER_HASH_H
#define MELOADER_HASH_H

#include <stdint.h>
#include <stddef.h>
#include "ocs/gpdma.h"
#include "devreg.h"

#define HASH_REG_MODE       (0x000)
#define HASH_REG_HASH       (0x004)
#define HASH_REG_CMD        (0x008)
#define HASH_REG_STS1       (0x050)
#define HASH_REG_STS2       (0x058)
#define HASH_REG_COUNTL     (0x060)
#define HASH_REG_COUNTH     (0x064)
#define HASH_REG_HMAC       (0x080)
#define HASH_REG_DATA       (0x600)

#define HASH_MODE_ALGO_MASK    (0x00030000)
#define HASH_MODE_ALGO_SHA1    (0x00000000)
#define HASH_MODE_ALGO_SHA512  (0x00010000)
#define HASH_MODE_ALGO_SHA256  (0x00020000)
#define HASH_MODE_HMAC         (0x00400000)

#define HASH_CMD_START_DATA    (1)
#define HASH_CMD_FLUSH_DATA    (2)

typedef struct {
    const char *name;
    uint8_t  hash_key[64];
    uint8_t  hash_buffer[64];
    uint8_t  hash_state[32];
    uint64_t hash_count;
    int      hash_dataptr;
    int      hash_stateptr;
    uint32_t hash_mode;
    uint32_t hash_status1;
    uint32_t hash_status2;
    uint8_t  hash_hmac_temp[64];
    gpdma_state hash_gpdma;
} ocs_hash;

int hash_read( ocs_hash *h, int addr, void *buffer, int count );
int hash_write( ocs_hash *h, int addr, const void *buffer, int count );
void hash_load_key( ocs_hash *h, void *data, size_t count );
int hash_get_result( ocs_hash *h, void *data, size_t count );
void hash_init( device_instance *parent, ocs_hash *h );

#endif //MELOADER_HASH_H
