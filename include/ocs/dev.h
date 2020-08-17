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
#include "ocs/gp.h"
#include "ocs/rsa.h"


typedef struct {
    uint32_t bdf;
    device_instance self;
    pci_func func;
    sks_inst sks;
    rsa_inst rsa;
    ocs_hash hash;
    ocs_aes aes;
    ocs_aes aes2;
    ocs_gp  gp;
    uint32_t sai;
} ocs_inst;

#endif //MELOADER_DEV_H
