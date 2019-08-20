//
// Created by pbx on 14/08/19.
//

#ifndef MELOADER_AES_H
#define MELOADER_AES_H

#include <stdint.h>
#include <stddef.h>
#include "ocs/gpdma.h"
#include "devreg.h"

typedef struct {
    const char *name;
    gpdma_state aes_gpdma;
} ocs_aes;

int aes_read( ocs_aes *a, int addr, void *buffer, int count );
int aes_write( ocs_aes *a, int addr, const void *buffer, int count );
void aes_load_key( ocs_aes *a, void *data, size_t count );
int aes_get_result( ocs_aes *a, void *data, size_t count );
void aes_init( device_instance *parent, ocs_aes *a, char c );

#endif //MELOADER_AES_H
