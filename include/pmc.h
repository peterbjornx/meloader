//
// Created by pbx on 01/09/19.
//

#ifndef MELOADER_PMC_H
#define MELOADER_PMC_H
#include "devreg.h"
#include "pci/device.h"

typedef struct {
    device_instance self;
    pci_simplefunc func;
    uint32_t sai;
    uint32_t d31id;
    uint32_t pps;
} pmc_inst;

#endif //MELOADER_PMC_H
