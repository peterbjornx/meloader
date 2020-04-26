//
// Created by pbx on 09/08/19.
//

#ifndef MELOADER_ATT_DEV_H
#define MELOADER_ATT_DEV_H

#include <pci/bus.h>
#include "devreg.h"
#include "gasket/att/regs.h"

typedef struct {
    att_regs regs;
    device_instance self;
    pci_bus  prim_bus;
    pci_func func;
    pci_func ret_func;
} att_inst;

int att_sb_read(att_inst *att, int addr, void *buffer, int count, int sai );
int att_sb_write(att_inst *att, int addr, const void *buffer, int count, int sai );
int att_sb_regs_read(att_inst *att, int addr, void *buffer, int count );
int att_sb_regs_write(att_inst *att, int addr, const void *buffer, int count );
int att_prim_read(att_inst *att, int addr, void *buffer, int count, int sai);
int att_prim_write(att_inst *att, int addr, const void *buffer, int count, int sai);
int att_prim_regs_read(att_inst *att, int addr, void *buffer, int count );
int att_prim_regs_write(att_inst *att, int addr, const void *buffer, int count );
att_sb_window *att_find_sb_win(att_inst *att, int address);


#endif //MELOADER_DEV_H
