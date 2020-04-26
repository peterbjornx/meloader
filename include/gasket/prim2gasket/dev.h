//
// Created by pbx on 10/08/19.
//

#ifndef MELOADER_DEV_H
#define MELOADER_DEV_H

#include "pci/bus.h"
#include "devreg.h"

typedef struct {
    pci_bus gasket_bus;
    device_instance self;
    pci_func func;
    pci_func ret_func;
} p2g_inst;

#endif //MELOADER_DEV_H
