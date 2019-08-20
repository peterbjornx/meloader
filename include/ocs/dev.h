//
// Created by pbx on 10/08/19.
//

#ifndef MELOADER_OCS_DEV_H
#define MELOADER_OCS_DEV_H

#include "ocs/hash/hash.h"
#include "devreg.h"
#include "pci/bus.h"
#include "ocs/sks.h"
#include "ocs/aes.h"

typedef struct {
    uint32_t bdf;
    device_instance self;
    pci_func func;
    sks_inst sks;
    ocs_hash hash;
    ocs_aes aes;
    uint32_t sai;
} ocs_inst;

#endif //MELOADER_DEV_H
