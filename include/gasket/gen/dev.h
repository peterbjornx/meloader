//
// Created by pbx on 23/04/20.
//

#ifndef MELOADER_GEN_DEV_H
#define MELOADER_GEN_DEV_H

#include "devreg.h"
#include "pci/device.h"

typedef struct {
    device_instance self;
    pci_simplefunc func;
    uint32_t sai;
} gen_inst;

#endif //MELOADER_DEV_H
