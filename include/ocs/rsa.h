//
// Created by pbx on 27/04/20.
//

#ifndef MELOADER_RSA_H
#define MELOADER_RSA_H
#include <stdint.h>
#include <devreg.h>
#include <gmp.h>
#include <x86_64-linux-gnu/gmp.h>

#define RSA_REG_COMMAND     (0x800)
#define RSA_REG_STATUS      (0x804)
#define RSA_REG_COUNT       (0x808)
#define RSA_REG_IRQ         (0x80C)
#define RSA_REG_EXPONENT(o) (0x000 + o)
#define RSA_REG_DATA(o)     (0x300 + o)
#define RSA_REG_MODULUS(o)  (0x500 + o)

#define RSA_CMD_SIZE(r)      ((r>>16u)&0x7F)
#define RSA_CMD_OPERATION(r) ((r>>24u)&0xF)
#define RSA_CMD_SMALLEXP     (1u << 29u)
#define RSA_STS_BUSY         (1u<<0u)

typedef struct {
    const char *name;
    uint32_t CMD;
    uint32_t STATUS;
    uint32_t COUNT;
    uint32_t IRQ;
    uint8_t  MODULUS [0x100];
    uint8_t  EXPONENT[0x100];
    uint8_t  DATA    [0x100];
    mpz_t    modulus_gmp;
    mpz_t    exponent_gmp;
    mpz_t    data_gmp;
    mpz_t    out_gmp;
} rsa_inst;

void rsa_init(  device_instance *parent, rsa_inst *sks );
int  rsa_read(  rsa_inst *sks, uint32_t addr, void *buffer, int count );
int  rsa_write( rsa_inst *sks, uint32_t addr, const void *buffer, int count );
void rsa_do_operation();
#endif //MELOADER_RSA_H
