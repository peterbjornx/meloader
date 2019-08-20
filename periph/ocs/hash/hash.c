//
// Created by pbx on 15/04/19.
//
#include "ocs/hash/hash.h"
#include "log.h"
#include "user/meloader.h"
#include "printf.h"
#include "ocs/hash/sha256.h"
#include "ocs/gpdma.h"
#include <string.h>
#include <stdio.h>

#define HASH_BASE (0xF510B000)
#define HASH_SIZE (0x1000)

/*
 Hash engine: 0x57
    +000: Mode
        Bits 16.18 Algo choice
            0 SHA1
            1 SHA512
            2 SHA256
        Bit 22:
            Use register C through 0x2C or SKS
        Bits 32..24
            0x00 if SHA512
            0xA8 for SHA256,SHA1

    +004: Hash Register
    +008: Command
        Bit 0: Start data load
        Bit 1: Finish data load
    +00C..+02C HMAC Key
    +050: Status 1
        Bit 2: Hashing done
    +058: Status 2
        Bit 0: Hashing busy
    +060:
    +064:
    +080:
        7 when HMAC
    +600: Data Input
    GPDMA (When source/dest size = 0, write to int buffer)
 */


void hash_round_init( ocs_hash *h ) {
    h->hash_count = 0;
    h->hash_status1 = 0;
    h->hash_status2 = 0;
    switch ( h->hash_mode & HASH_MODE_ALGO_MASK ) {
        case HASH_MODE_ALGO_SHA256:
            sha256_init_bytes( h->hash_state );
            break;
        default:
            log( LOG_ERROR, h->name, "hash algorithm unimpl 0x%08x", h->hash_mode);
            break;

    }
}

void hash_round_process( ocs_hash *h ) {
    h->hash_stateptr = 0;
    switch( h->hash_mode & HASH_MODE_ALGO_MASK ) {
        case HASH_MODE_ALGO_SHA256:
            sha256_transform_bytes(h->hash_state, h->hash_buffer);
            break;
        default:
            log( LOG_ERROR, h->name, "hash algorithm unimpl 0x%08x", h->hash_mode);
            break;
    }

}

void hash_round_finish(ocs_hash *h, int r) {
    uint64_t lenInBits;
    uint32_t curBufferPos;
    unsigned i;

    lenInBits = (h->hash_count << 3);
    curBufferPos = (uint32_t)h->hash_count & 0x3F;
    switch( h->hash_mode & HASH_MODE_ALGO_MASK ) {
        case HASH_MODE_ALGO_SHA256:
            h->hash_buffer[curBufferPos++] = 0x80;
            while (curBufferPos != (64 - 8))
            {
                curBufferPos &= 0x3F;
                if (curBufferPos == 0)
                    hash_round_process(h);
                h->hash_buffer[curBufferPos++] = 0;
            }
            for (i = 0; i < 8; i++)
            {
                h->hash_buffer[curBufferPos++] = (unsigned char)(lenInBits >> 56);
                lenInBits <<= 8;
            }
            hash_round_process(h);
            break;
        default:
            log(LOG_FATAL, h->name, "hash algorithm unimpl 0x%08x", h->hash_mode);
            break;
    }
    if ( h->hash_mode & HASH_MODE_HMAC && !r ) {
        memcpy( h->hash_hmac_temp, h->hash_state, 32 );
        hash_round_init(h);
        for ( i = 0; i < 64; i++ )
            h->hash_buffer[i] = (uint8_t) (h->hash_key[i] ^ 0x5C);
        h->hash_count += 64;
        hash_round_process(h);
        memcpy( h->hash_buffer, h->hash_hmac_temp, 32 );
        h->hash_count += 32;
        hash_round_finish(h, 1);
    }
    h->hash_status1 |= 4;
}

void hash_hmac_round1_init(ocs_hash *h) {
    int i;
    for ( i = 0; i < 64; i++ )
        h->hash_buffer[i] = (uint8_t) (h->hash_key[i] ^ 0x36);
    h->hash_count = 64;
    hash_round_process(h);
}

void hash_do_command( ocs_hash *h, uint32_t cmd ) {
    switch ( cmd ) {
        case HASH_CMD_START_DATA:
            log(LOG_TRACE, h->name, "Start data index:%i pos:%i", h->hash_count, h->hash_count & 63);
            h->hash_count &= ~63;
            if ( h->hash_mode & HASH_MODE_HMAC )
                hash_hmac_round1_init(h);
            break;
        case HASH_CMD_FLUSH_DATA:
            log(LOG_TRACE, h->name, "Flush data index:%i pos:%i", h->hash_count, h->hash_count & 63);
            hash_round_finish(h,0);
            break;
        default:
            log(LOG_ERROR, h->name, "[hash] unknown command 0x%08x", cmd);
            break;
    }
}

void hash_dma_write( ocs_hash *h, const void *buffer, size_t count ) {
    int rc,bp;
    if (count <= 0) {
        log(LOG_ERROR, h->name, "[hash] bad data write start:%i count:%i",
                   h->hash_dataptr, count);
        return;
    }
    while (count) {
        rc = count;
        bp = (int) (h->hash_count & 63);
        if (rc + bp > 64)
            rc = 64 - bp;
        memcpy(h->hash_buffer + bp, buffer, (size_t) rc);
        h->hash_count += rc;
        count -= rc;
        buffer += rc;
        if ((h->hash_count & 63) == 0)
            hash_round_process(h);
    }
}

int hash_read( ocs_hash *h, int addr, void *buffer, int count ) {
    int i = 0;
    uint32_t *buf = buffer;
    if ( addr < 0 || addr >= HASH_SIZE )
        return 0;
    if ( count != 4 || (addr & 3) ) {
        log(LOG_ERROR, h->name, "read misaligned 0x%03x count:%i", addr, count);
        return 1;
    }
    if ( addr == HASH_REG_STS1 ) {
        *buf = h->hash_status1;
    } else if ( addr == HASH_REG_STS2 ) {
        *buf = h->hash_status2;
    } else if ( addr == HASH_REG_COUNTL ) {
        memcpy(buffer, &h->hash_count, (size_t) count);
    } else if ( addr == HASH_REG_COUNTH ) {
        memcpy(buffer, ((void *)&h->hash_count) + 4, (size_t) count);
    } else if ( addr == HASH_REG_HASH ) {
        if ( h->hash_stateptr + count > 32 ) {
            log(LOG_ERROR, h->name, "bad state read off:%i count:%i", addr, count);
            return 1;
        }
        memcpy(buffer, h->hash_state + h->hash_stateptr, (size_t) count);
        log(LOG_TRACE, h->name, "read hash count:%i val: 0x%08x", count, *buf);
        h->hash_stateptr += count;
    } else if ( !gpdma_read( &h->hash_gpdma, addr, buffer, count) )
        log(LOG_ERROR, h->name, "read  unknown 0x%03x count:%i val: 0x%08x", addr, count, *buf);
    return 1;
}

int hash_write( ocs_hash *h, int addr, const void *buffer, int count ) {
    const uint32_t *buf = buffer;
    if ( addr < 0 || addr >= HASH_SIZE )
        return 0;
    if ( addr == HASH_REG_DATA ) {
        hash_dma_write( h, buffer, count );
        return 1;
    } else if ( count != 4 || (addr & 3) ) {
        log(LOG_ERROR, h->name, "write misaligned 0x%03x count:%i", addr, count);
        return 1;
    }
    if ( addr == HASH_REG_MODE ) {
        log(LOG_TRACE, h->name, "write MODE 0x%08x", *buf);
        h->hash_mode = *buf;
        h->hash_stateptr = 0;
        hash_round_init(h);
    } else if ( addr == HASH_REG_CMD ) {
        log(LOG_TRACE, h->name, "write CMD 0x%08x", *buf);
        hash_do_command( h, *buf );
    } else if ( addr == HASH_REG_HASH ) {
        if ( h->hash_stateptr + count > 32 ) {
            log(LOG_ERROR, h->name, "bad state write off:%i count:%i", addr, count);
            return 1;
        }
        log(LOG_TRACE, h->name, "write hash count:%i val: 0x%08x", count, *buf);
        memcpy( h->hash_state + h->hash_stateptr, buffer, (size_t) count);
        h->hash_stateptr += count;
    } else if ( addr == HASH_REG_COUNTL ) {
        memcpy(&h->hash_count, buffer, (size_t) count);
    } else if ( addr == HASH_REG_COUNTH ) {
        memcpy(((void *)&h->hash_count) + 4, buffer, (size_t) count);
    } else if ( addr == HASH_REG_STS1 ) {
        h->hash_status1 &= ~*buf;
    } else if ( addr == HASH_REG_STS2 ) {
        h->hash_status2 &= ~*buf;
    } else if ( !gpdma_write( &h->hash_gpdma, addr, buffer, count) )
        log(LOG_ERROR, h->name, "write unknown 0x%03x count:%i val: 0x%08x", addr, count, *buf);

    return 1;
}


void hash_dma_read( ocs_hash *h, void *data, size_t size ) {
    log(LOG_ERROR, h->name, "bad dma read from hash!!!");
}

void hash_load_key( ocs_hash *h, void *data, size_t count ) {
    memset( h->hash_key, 0, count );
    memcpy( h->hash_key, data, count );
}

int hash_get_result( ocs_hash *h, void *data, size_t count ){
    memcpy( data, h->hash_state, count );
    return 1;
}

void hash_init( device_instance *parent, ocs_hash *h ) {
    char name[160];
    memset( h, 0, sizeof(ocs_hash) );
    snprintf( name, 160, "%s_hcu", parent->name );
    gpdma_init( &h->hash_gpdma );
    h->name = strdup(name);
    h->hash_gpdma.impl = h;
    h->hash_gpdma.int_read = hash_dma_read;
    h->hash_gpdma.int_write = hash_dma_write;
}