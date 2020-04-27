//
// Created by pbx on 27/04/20.
//
#include <stdio.h>
#include <string.h>
#include <ocs/rsa.h>
#include <log.h>
#include <assert.h>


int rsa_read(rsa_inst *rsa, uint32_t addr, void *buffer, int count ) {
    int i = 0;
    uint32_t *buf = buffer;
    if ( count != 4 || (addr & 3) ) {
        log(LOG_ERROR, rsa->name, "read misaligned 0x%03x count:%i", addr, count);
        return 1;
    }if ( addr == RSA_REG_COMMAND ) {
        log(LOG_INFO, rsa->name, "read  rsa_command  = 0x%08x", *buf);
        *buf = rsa->CMD;
    } else if ( addr == RSA_REG_STATUS ) {
        *buf = rsa->STATUS;
        log(LOG_TRACE, rsa->name, "read  rsa_status   = 0x%08x", *buf);
    } else if ( addr == RSA_REG_COUNT ) {
        *buf = rsa->COUNT;
        log(LOG_INFO, rsa->name, "read  rsa_count    = 0x%08x", *buf);
    } else if ( addr == RSA_REG_IRQ ) {
        *buf = rsa->IRQ;
        log(LOG_INFO, rsa->name, "read  rsa_irq      = 0x%08x", *buf);
    } else if ( addr >= RSA_REG_DATA(0) && (addr + count) <= RSA_REG_DATA(0x100) ) {
        i = addr - RSA_REG_DATA(0);
        memcpy( buffer, rsa->DATA + i, count );
    }  else if ( addr >= RSA_REG_EXPONENT(0) && (addr + count) <= RSA_REG_EXPONENT(0x100) ) {
        i = addr - RSA_REG_EXPONENT(0);
        memcpy( buffer, rsa->EXPONENT + i, count );
    } else if ( addr >= RSA_REG_MODULUS(0) && (addr + count) <= RSA_REG_MODULUS(0x100) ) {
        i = addr - RSA_REG_MODULUS(0);
        memcpy( buffer, rsa->MODULUS + i, count );
    }  else
        log(LOG_WARN, rsa->name, "read  0x%03x count:%i", addr, count);
    return 1;
}


int rsa_write( rsa_inst *rsa, uint32_t addr, const void *buffer, int count ) {
    const uint32_t *buf = buffer;
    int i;
    if ( count != 4 || (addr & 3) ) {
        log(LOG_ERROR, rsa->name, "write misaligned 0x%03x count:%i", addr, count);
        return 1;
    }if ( addr == RSA_REG_COMMAND ) {
        log(LOG_TRACE, rsa->name, "write rsa_command  = 0x%08x", *buf);
        rsa->CMD = *buf;
    } else if ( addr == RSA_REG_STATUS ) {
        rsa->STATUS = *buf;
        log(LOG_INFO, rsa->name, "write rsa_status   = 0x%08x", *buf);
    } else if ( addr == RSA_REG_COUNT ) {
        rsa->COUNT = *buf;
        log(LOG_INFO, rsa->name, "write rsa_count    = 0x%08x", *buf);
    } else if ( addr == RSA_REG_IRQ ) {
        rsa->IRQ = *buf;
        log(LOG_TRACE, rsa->name, "write rsa_irq      = 0x%08x", *buf);
    } else if ( addr >= RSA_REG_DATA(0) && (addr + count) <= RSA_REG_DATA(0x100) ) {
        i = addr - RSA_REG_DATA(0);
        memcpy( rsa->DATA + i, buffer, count );
    }  else if ( addr >= RSA_REG_EXPONENT(0) && (addr + count) <= RSA_REG_EXPONENT(0x100) ) {
        i = addr - RSA_REG_EXPONENT(0);
        memcpy( rsa->EXPONENT + i, buffer, count );
    } else if ( addr >= RSA_REG_MODULUS(0) && (addr + count) <= RSA_REG_MODULUS(0x100) ) {
        i = addr - RSA_REG_MODULUS(0);
        memcpy( rsa->MODULUS + i, buffer, count );
    }  else
        log(LOG_WARN, rsa->name, "write 0x%03x count:%i", addr, count);
    rsa_do_operation( rsa );
    return 1;
}
static void reverse_arr(uint32_t arr[], int count)
{
    int start = 0;
    int end = count - 1;
    while (start < end)
    {
        int temp = arr[start];
        arr[start] = arr[end];
        arr[end] = temp;
        start++;
        end--;
    }
}

void rsa_do_operation( rsa_inst *rsa ) {
    int op       = RSA_CMD_OPERATION(rsa->CMD);
    int count_dw = RSA_CMD_SIZE(rsa->CMD);
    if ( op == 6 ) {
        memset( rsa->DATA, 0, sizeof rsa->DATA );
        memset( rsa->EXPONENT, 0, sizeof rsa->EXPONENT );
        memset( rsa->MODULUS, 0, sizeof rsa->MODULUS );
        log(LOG_INFO, rsa->name, "Soft reset !");
        rsa->CMD = 0;
    } else if (op == 1 ) {
        reverse_arr( rsa->DATA, count_dw );
        reverse_arr( rsa->EXPONENT, count_dw );
        reverse_arr( rsa->MODULUS, count_dw );
        mpz_import( rsa->data_gmp, count_dw, 0, sizeof(uint32_t), -1, 0, rsa->DATA );
        mpz_import( rsa->exponent_gmp, count_dw, 0, sizeof(uint32_t), -1, 0, rsa->EXPONENT );
        mpz_import( rsa->modulus_gmp, count_dw, 0, sizeof(uint32_t), -1, 0, rsa->MODULUS );
        mpz_powm( rsa->out_gmp, rsa->data_gmp, rsa->exponent_gmp, rsa->modulus_gmp );
        size_t rc;
        memset( rsa->DATA, 0, sizeof rsa->DATA );
        mpz_export( rsa->DATA, &rc , 0, sizeof(uint32_t), -1, 0, rsa->out_gmp );
        reverse_arr( rsa->DATA, count_dw );
        logassert( rc <= (sizeof rsa->DATA / sizeof(uint32_t)), rsa->name, "Overly large RSA result: %x", rc);
    } else if ( op != 0 ) {
            log(LOG_INFO, rsa->name, "Unknown command %i, with size: 0x%x  !",op, count_dw);
            rsa->CMD = 0;
        }

}

void rsa_init( device_instance *parent, rsa_inst *rsa ) {
    int i;
    char name[160];
    snprintf( name, 160, "%s_rsa", parent->name );
    rsa->name = strdup(name);
    mpz_init( rsa->data_gmp );
    mpz_init( rsa->modulus_gmp );
    mpz_init( rsa->exponent_gmp );
    mpz_init( rsa->out_gmp );
}
