//
// Created by pbx on 01/09/19.
//]

#ifndef MELOADER_EXI_H
#define MELOADER_EXI_H

#include "devreg.h"
#include "sideband.h"

typedef struct {
    device_instance self;
    sideband_dev sb;
    int sai;
    uint32_t emecc;
    uint32_t ectrl;
} exi_inst;

#endif //MELOADER_DFXAGG_H

