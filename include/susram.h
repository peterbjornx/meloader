//
// Created by pbx on 01/09/19.
//

#ifndef MELOADER_SUSRAM_H
#define MELOADER_SUSRAM_H

#include "devreg.h"
#include "sideband.h"

typedef struct {
    device_instance self;
    sideband_dev sb;
    int sai;
} susram_inst;

#endif //MELOADER_SUSRAM_H
