//
// Created by pbx on 23/08/19.
//

#ifndef MELOADER_FUSECTL_H
#define MELOADER_FUSECTL_H

#include "devreg.h"
#include "sideband.h"

typedef struct {
    device_instance self;
    sideband_dev    sb;
    int sai;
    uint32_t control;
    uint32_t intel_global_ctrl;
    uint32_t intel_kar;
    uint32_t intel_address;
    uint32_t intel_size;
    uint32_t intel_fsm_ctrl;
    uint32_t intel_hd_counter;
    uint32_t intel_debug_status;
    uint32_t intel_status_1;
    uint32_t intel_status_2;
    uint32_t intel_status_3;
    uint32_t unknown_1020;
    uint32_t unknown_1024;
} fusectl_inst;

#endif //MELOADER_FUSE_CONTROLLER_H
