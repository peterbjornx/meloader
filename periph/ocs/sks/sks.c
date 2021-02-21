//
// Created by pbx on 15/04/19.
//
#include "user/meloader.h"
#include "ocs/sks.h"
#include "log.h"
#include <string.h>
#include <stdio.h>

int sks_read( sks_inst *sks, uint32_t addr, void *buffer, int count ) {
    int i = 0;
    uint32_t *buf = buffer;
    if ( count != 4 || (addr & 3) ) {
        log(LOG_ERROR, sks->name, "read misaligned 0x%03x count:%i", addr, count);
        return 1;
    }
    if ( addr == SKS_REG_SLOT ) {
        *buf = sks->sks_slot;
        log(LOG_TRACE, sks->name, "read  sks_slot     = 0x%08x", *buf);
    } else if ( addr == SKS_REG_COMMAND ) {
        log(LOG_TRACE, sks->name, "read  sks_command  = 0x%08x", *buf);
        *buf = sks->sks_command;
    } else if ( addr == SKS_REG_PERMISSION ) {
        *buf = sks->sks_permission;
        log(LOG_TRACE, sks->name, "read  sks_permission  = 0x%08x", *buf);
    }  else if ( addr == SKS_REG_STATUS ) {
        *buf = sks->sks_status;
        log(LOG_TRACE, sks->name, "read  sks_status   = 0x%08x", *buf);
    } else if ( addr >= SKS_REG_DATA(0) && addr < SKS_REG_DATA(32) ) {
        i = addr - SKS_REG_DATA(0);
        log(LOG_WARN, sks->name, "SKS readback attempt! off:%i count:%i", i, count);
        *buf = 0;
    } else if ( addr >= SKS_REG_ATR(0) && addr < SKS_REG_ATR(22) ) {
        i = (addr - SKS_REG_ATR(0)) / 4;
        *buf = sks->sks_keys[i].key_atr;
        log(LOG_TRACE, sks->name, "read slot[%i].atr1 = 0x%08x", i, *buf);
    } else
        log(LOG_WARN, sks->name, "read  0x%03x count:%i", addr, count);
    return 1;
}


int sks_write( sks_inst *sks, uint32_t addr, const void *buffer, int count ) {
    const uint32_t *buf = buffer;
    int i;
    if ( addr >= SKS_REG_DATA(0) && addr < SKS_REG_DATA(32) ) {
        i = addr - SKS_REG_DATA(0);
        if ( count < 0 || (i + count) > 32 ) {
            log(LOG_ERROR, sks->name, "bad data write start:%i count:%i", i, count);
            return 1;
        }
        memcpy( sks->sks_databuf + i, buffer, (size_t) count);
    } else if ( count != 4 || (addr & 3) ) {
        log(LOG_ERROR, sks->name, "write misaligned 0x%03x count:%i", addr, count);
        return 1;
    }
    if ( addr == SKS_REG_SLOT ) {
        sks->sks_slot = *buf;
        log(LOG_TRACE, sks->name, "write sks_slot     = 0x%08x", *buf);
    } else if ( addr == SKS_REG_COMMAND ) {
        sks->sks_command = *buf;
        log(LOG_TRACE, sks->name, "write sks_command  = 0x%08x", *buf);
    } else if ( addr == SKS_REG_PERMISSION ) {
        sks->sks_permission = *buf;
        int  perm = (sks->sks_permission >> SKS_PERM_LEVEL) & 3;
        int  aesa = (sks->sks_permission >> SKS_PERM_LEVEL_AESA) & 3;
        int  aesp = (sks->sks_permission >> SKS_PERM_LEVEL_AESP) & 3;
        int  hcu  = (sks->sks_permission >> SKS_PERM_LEVEL_HCU) & 3;
        log(LOG_TRACE, sks->name, "write sks_permission  = 0x%08x : C:%i A:%i P: %i H:%i", sks->sks_permission,perm,aesa,aesp,hcu);
    } else if ( addr == SKS_REG_STATUS ) {
        sks->sks_status &= ~*buf;
        log(LOG_TRACE, sks->name, "write sks_status   = 0x%08x", *buf);
    } else if ( addr >= SKS_REG_ATR(0) && addr < SKS_REG_ATR(22) ) {
        i = (addr - SKS_REG_ATR(0)) / 4;
        sks->sks_keys[i].key_atr = *buf;
        log(LOG_TRACE, sks->name, "write slot[%i].atr1 = 0x%08x", i, *buf);
    } else
        log(LOG_TRACE, sks->name, "write unknown 0x%03x count:%i val: 0x%08x", addr, count, *buf);
    sks_do_operation( sks );
    return 1;
}

void sks_do_operation( sks_inst *sks ) {
    size_t keysize;
    if ( ~sks->sks_command & SKS_CMD_GO )
        return;
    sks->sks_command &= ~SKS_CMD_GO; //XXX: Guessed this bit is a GO bit
    sks->sks_status  |=  SKS_STS_BUSY;
    if ( sks->sks_slot < 0 || sks->sks_slot > 22 )
        goto err;
    keysize = sks->sks_slot >= 11 ? 32 : 16;
    if ( sks->sks_command & SKS_CMD_GETSET ) {
        switch( sks->sks_command & SKS_CMD_GETSRC_MASK ) {
            case SKS_CMD_SOURCE_CPU:
                memcpy( sks->sks_keys[sks->sks_slot].key_data, sks->sks_databuf, keysize );
                memset( sks->sks_databuf, 0, sizeof sks->sks_databuf );
                break;
            case SKS_CMD_SOURCE_AES:
                if (! sks->aes_get_result( sks->aes, sks->sks_keys[sks->sks_slot].key_data, keysize ) ) {
                    log(LOG_ERROR, sks->name, "AES output not ready!");
                    goto err;
                }
                break;
            case SKS_CMD_SOURCE_HASH:
                if (! sks->hash_get_result( sks->hash, sks->sks_keys[sks->sks_slot].key_data, keysize ) ) {
                    log( LOG_ERROR, sks->name, "Hash output not ready!");
                    goto err;
                }
                break;
            default:
                goto err;
        }
        sks->sks_keys[sks->sks_slot].key_atr |= SKS_ATR_KEY_VALID;
    } else if ( sks->sks_command & SKS_CMD_TARGET_HMAC ) {
        log(LOG_INFO, sks->name, "SKS load key %i to SHA!",sks->sks_slot);
        sks->hash_load_key( sks->hash, sks->sks_keys[sks->sks_slot].key_data, keysize );
    } else if ( sks->sks_command & SKS_CMD_TARGET_AES ) {
        log(LOG_INFO, sks->name, "SKS load key %i to AES!",sks->sks_slot);
        sks->aes_load_key( sks->aes, sks->sks_keys[sks->sks_slot].key_data, keysize );
    }
    sks->sks_status &= ~SKS_STS_BUSY;
    return;
err:
    sks->sks_status &= ~SKS_STS_BUSY;
    sks->sks_status |= SKS_STS_ERRR;
}

void sks_init( device_instance *parent, sks_inst *sks ) {
    int i;
    char name[160];
    memset( sks->sks_keystore, 0, sizeof sks->sks_keystore );
    snprintf( name, 160, "%s_sks", parent->name );
    sks->name = strdup(name);
    for ( i = 0; i < 11; i++ ) {
        sks->sks_keys[i].key_data = sks->sks_keystore + 16 * i;
        sks->sks_keys[i].key_atr  = 0;
    }
    for ( i = 0; i < 11; i++ ) {
        sks->sks_keys[i + 11].key_data = sks->sks_keystore + 16 * 11 + 32 * i;
        sks->sks_keys[i + 11].key_atr  = 0;
    }
    //TODO: Add configuration for initial key slot values
    sks->sks_keys[21].key_atr |= SKS_ATR_KEY_VALID;

}
