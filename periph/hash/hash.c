//
// Created by pbx on 15/04/19.
//
#include "meloader.h"
#include "printf.h"
#include "hash/sha256.h"
#include "gpdma.h"

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

uint8_t  hash_key[64];
uint8_t  hash_buffer[64];
uint8_t  hash_state[32];
uint64_t hash_count;
int      hash_dataptr = 0;
int      hash_stateptr = 0;
uint32_t hash_mode = 0;
uint32_t hash_status1 = 0;
uint32_t hash_status2 = 0;
uint8_t  hash_hmac_temp[64];
gpdma_state hash_gpdma;

void hash_round_init() {
    hash_count = 0;
    hash_status1 = 0;
    hash_status2 = 0;
    switch ( hash_mode & HASH_MODE_ALGO_MASK ) {
        case HASH_MODE_ALGO_SHA256:
            sha256_init_bytes(hash_state);
            break;
        default:
            mel_printf("[hash] hash algorithm unimpl 0x%08x\n", hash_mode);
            break;

    }
}

void hash_round_process() {
    hash_stateptr = 0;
    switch( hash_mode & HASH_MODE_ALGO_MASK ) {
        case HASH_MODE_ALGO_SHA256:
            sha256_transform_bytes(hash_state, hash_buffer);
            break;
        default:
            mel_printf("[hash] hash algorithm unimpl 0x%08x\n", hash_mode);
            break;
    }

}

void hash_round_finish(int r) {
    uint64_t lenInBits;
    uint32_t curBufferPos;
    unsigned i;

    lenInBits = (hash_count << 3);
    curBufferPos = (uint32_t)hash_count & 0x3F;
    switch( hash_mode & HASH_MODE_ALGO_MASK ) {
        case HASH_MODE_ALGO_SHA256:
            hash_buffer[curBufferPos++] = 0x80;
            while (curBufferPos != (64 - 8))
            {
                curBufferPos &= 0x3F;
                if (curBufferPos == 0)
                    hash_round_process();
                hash_buffer[curBufferPos++] = 0;
            }
            for (i = 0; i < 8; i++)
            {
                hash_buffer[curBufferPos++] = (unsigned char)(lenInBits >> 56);
                lenInBits <<= 8;
            }
            hash_round_process();
            break;
        default:
            mel_printf("[hash] hash algorithm unimpl 0x%08x\n", hash_mode);
            break;
    }
    if ( hash_mode & HASH_MODE_HMAC && !r ) {
        memcpy( hash_hmac_temp, hash_state, 32 );
        hash_round_init();
        for ( i = 0; i < 64; i++ )
            hash_buffer[i] = (uint8_t) (hash_key[i] ^ 0x5C);
        hash_count += 64;
        hash_round_process();
        memcpy( hash_buffer, hash_hmac_temp, 32 );
        hash_count += 32;
        hash_round_finish(1);
    }
    hash_status1 |= 4;
}

void hash_hmac_round1_init() {
    int i;
    for ( i = 0; i < 64; i++ )
        hash_buffer[i] = (uint8_t) (hash_key[i] ^ 0x36);
    hash_count = 64;
    hash_round_process();
}

void hash_do_command( uint32_t cmd ) {
    switch ( cmd ) {
        case HASH_CMD_START_DATA:
            //mel_printf("[hash] Start data index:%i pos:%i\n", hash_count, hash_count & 63);
            hash_count &= ~63;
            if ( hash_mode & HASH_MODE_HMAC )
                hash_hmac_round1_init();
            break;
        case HASH_CMD_FLUSH_DATA:
            //mel_printf("[hash] Flush data index:%i pos:%i\n", hash_count, hash_count & 63);
            hash_round_finish(0);
            break;
        default:
            mel_printf("[hash] unknown command 0x%08x\n", cmd);
            break;
    }
}

void hash_dma_write( const void *buffer, size_t count ) {
    int rc,bp;
    if (count <= 0) {
        mel_printf("[hash] bad data write start:%i count:%i\n",
                   hash_dataptr, count);
        return;
    }
    while (count) {
        rc = count;
        bp = (int) (hash_count & 63);
        if (rc + bp > 64)
            rc = 64 - bp;
        memcpy(hash_buffer + bp, buffer, (size_t) rc);
        hash_count += rc;
        count -= rc;
        buffer += rc;
        if ((hash_count & 63) == 0)
            hash_round_process();
    }
}

int hash_read(int addr, void *buffer, int count ) {
    int i = 0;
    uint32_t *buf = buffer;
    addr -= HASH_BASE;
    if ( addr < 0 || addr >= HASH_SIZE )
        return 0;
    if ( count != 4 || (addr & 3) ) {
        mel_printf("[hash] read misaligned 0x%03x count:%i\n", addr, count);
        return 1;
    }
    if ( addr == HASH_REG_STS1 ) {
        *buf = hash_status1;
    } else if ( addr == HASH_REG_STS2 ) {
        *buf = hash_status2;
    } else if ( addr == HASH_REG_COUNTL ) {
        memcpy(buffer, &hash_count, (size_t) count);
    } else if ( addr == HASH_REG_COUNTH ) {
        memcpy(buffer, ((void *)&hash_count) + 4, (size_t) count);
    } else if ( addr == HASH_REG_HASH ) {
        if ( hash_stateptr + count > 32 ) {
            mel_printf("[hash] bad state read off:%i count:%i\n", addr, count);
            return 1;
        }
        memcpy(buffer, hash_state + hash_stateptr, (size_t) count);
        //mel_printf("[hash] read hash count:%i val: 0x%08x\n", count, *buf);
        hash_stateptr += count;
    } else if ( !gpdma_read( &hash_gpdma, addr, buffer, count) )
        mel_printf("[hash] read  unknown 0x%03x count:%i val: 0x%08x\n", addr, count, *buf);
    return 1;
}

int hash_write( int addr, const void *buffer, int count ) {
    const uint32_t *buf = buffer;
    addr -= HASH_BASE;
    if ( addr < 0 || addr >= HASH_SIZE )
        return 0;
    if ( addr == HASH_REG_DATA ) {
        hash_dma_write( buffer, count );
        return 1;
    } else if ( count != 4 || (addr & 3) ) {
        //mel_printf("[hash] write misaligned 0x%03x count:%i\n", addr, count);
        return 1;
    }
    if ( addr == HASH_REG_MODE ) {
        //mel_printf("[hash] write MODE 0x%08x\n", *buf);
        hash_mode = *buf;
        hash_stateptr = 0;
        hash_round_init();
    } else if ( addr == HASH_REG_CMD ) {
        //mel_printf("[hash] write CMD 0x%08x\n", *buf);
        hash_do_command( *buf );
    } else if ( addr == HASH_REG_HASH ) {
        if ( hash_stateptr + count > 32 ) {
            mel_printf("[hash] bad state write off:%i count:%i\n", addr, count);
            return 1;
        }
        //mel_printf("[hash] write hash count:%i val: 0x%08x\n", count, *buf);
        memcpy( hash_state + hash_stateptr, buffer, (size_t) count);
        hash_stateptr += count;
    } else if ( addr == HASH_REG_COUNTL ) {
        memcpy(&hash_count, buffer, (size_t) count);
    } else if ( addr == HASH_REG_COUNTH ) {
        memcpy(((void *)&hash_count) + 4, buffer, (size_t) count);
    } else if ( addr == HASH_REG_STS1 ) {
        hash_status1 &= ~*buf;
    } else if ( addr == HASH_REG_STS2 ) {
        hash_status2 &= ~*buf;
    } else if ( !gpdma_write( &hash_gpdma, addr, buffer, count) )
        mel_printf("[hash] write unknown 0x%03x count:%i val: 0x%08x\n", addr, count, *buf);

    return 1;
}


void hash_dma_read( void *data, size_t size ) {
    mel_printf("[hash] bad dma read from hash!!!\n");

}

void hash_load_key( void *data, size_t count ) {
    memset( hash_key, 0, count );
    memcpy( hash_key, data, count );
}

int hash_get_result( void *data, size_t count ){
    memcpy( data, hash_state, count );
    return 1;
}


mmio_periph hash_mmio = {
        .write = hash_write,
        .read = hash_read
};

void hash_init(){
    gpdma_init( &hash_gpdma );
    hash_gpdma.int_read = hash_dma_read;
    hash_gpdma.int_write = hash_dma_write;
}

void hash_install() {
    hash_init();
    krnl_periph_reg( &hash_mmio );
}