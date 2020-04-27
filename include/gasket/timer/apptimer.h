//
// Created by pbx on 26/04/20.
//

#ifndef MELOADER_APPTIMER_H
#define MELOADER_APPTIMER_H

#include <fsb.h>
#include <pmc.h>

#define APPTIMER_REG_TIME_NOW (0x08)

typedef struct __attribute__((packed)) {
    uint64_t reserved_0000;
    uint64_t TIME_NOW;      /* 0008 */
    uint64_t TIME_ALARM;    /* 0010 */
    uint32_t CONTROL;       /* 0018 */
    uint32_t POWER;         /* 001C */
    uint8_t  reserved_0020[0x4FF0];
    uint32_t UNK_5000;
} apptimer_bar0;

typedef struct {
    device_instance self;
    pci_simplefunc func;
    uint32_t sai;
    apptimer_bar0 bar0_regs;
} apptimer_inst;

#endif //MELOADER_APPTIMER_H
