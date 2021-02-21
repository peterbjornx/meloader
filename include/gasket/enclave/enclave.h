//
// Created by pbx on 01/09/19.
//

#ifndef MELOADER_ENCLAVE_H
#define MELOADER_ENCLAVE_H
#include "devreg.h"
#include "pci/device.h"

typedef struct {
    device_instance self;
    pci_simplefunc func;
    uint32_t sai;
} enclave_inst;

#endif //MELOADER_HECI_H
