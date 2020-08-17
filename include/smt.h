//
// Created by pbx on 01/09/19.
//

#ifndef MELOADER_SMT_H
#define MELOADER_SMT_H
#include "devreg.h"
#include "pci/device.h"

typedef struct {
    device_instance self;
    pci_simplefunc func;
    uint32_t sai;
    uint32_t GR_GCTRL, MSTR_MCTRL, MSTR_MDBA, MSTR_MDS, MSTR_MSTS;
    uint8_t dma_buf[128];
} smt_inst;

/* General Control Register (GCTRL) bit definitions */
#define SMT_GCTRL_TRST	0x04u	/* Target Reset */
#define SMT_GCTRL_KILL	0x08u	/* Kill */
#define SMT_GCTRL_SRST	0x40u	/* Soft Reset */

/* Master Control Register (MCTRL) bit definitions */
#define SMT_MCTRL_SS	0x01u		/* Start/Stop */
#define SMT_MCTRL_MEIE	0x10u		/* Master Error Interrupt Enable */
#define SMT_MCTRL_FMHP	0x00ff0000u	/* Firmware Master Head Ptr (FMHP) */

/* Master Status Register (MSTS) bit definitions */
#define SMT_MSTS_HMTP	0xff0000	/* HW Master Tail Pointer (HMTP) */
#define SMT_MSTS_MIS	0x20		/* Master Interrupt Status (MIS) */
#define SMT_MSTS_MEIS	0x10		/* Master Error Int Status (MEIS) */
#define SMT_MSTS_IP 	0x01		/* In Progress */

#endif //MELOADER_PMC_H
