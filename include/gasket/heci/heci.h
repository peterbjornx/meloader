//
// Created by pbx on 01/09/19.
//

#ifndef MELOADER_HECI_H
#define MELOADER_HECI_H
#include "devreg.h"
#include "pci/device.h"

typedef struct {
    device_instance self;
    pci_simplefunc func;
    uint32_t sai;
    uint32_t cse_fs;
    uint32_t cse_gs1;
    uint32_t cse_gs2;
    uint32_t cse_gs3;
    uint32_t cse_gs4;
    uint32_t cse_gs5;
    uint32_t cse_shdw1;
    uint32_t cse_shdw2;
    uint32_t cse_cuba;
} heci_inst;

void heci_handle_gs1_change( heci_inst *i, uint32_t newval );
void heci_handle_fs_change( heci_inst *i, uint32_t newval );
#endif //MELOADER_HECI_H
