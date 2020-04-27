//
// Created by pbx on 07/08/19.
//

#ifndef MELOADER_EMU_H
#define MELOADER_EMU_H

#include <stdint.h>
#include "fsb.h"
#include "pci/bus.h"
#include "misa/regs.h"

typedef struct{
    device_instance  self;
    mia_fsb          *fsb;
    misa_regs        registers;
    pci_bus          bus;
    pci_func         bus_target;
    device_instance *cpu;
    int              sai;
    uint64_t         bdf;
    int              bunit_user;
    uint32_t         sram_size;
    uint8_t         *sram_data;
    uint32_t         rom_size;
    uint8_t         *rom_data;
} misa_inst;

void misa_fsb_mem_write( device_instance *sa, uint32_t addr, const void *buffer, size_t size );
void misa_fsb_mem_read ( device_instance *sa, uint32_t addr,       void *buffer, size_t size );
void misa_fsb_io_write ( device_instance *sa, uint32_t addr, const void *buffer, size_t size );
void misa_fsb_io_read  ( device_instance *sa, uint32_t addr,       void *buffer, size_t size );
void misa_fsb_init( misa_inst *sa, const cfg_section *section );

void misa_aunit_upstream_write( misa_inst *sa, uint32_t addr, const void *buffer, size_t size );
void misa_aunit_upstream_read ( misa_inst *sa, uint32_t addr,       void *buffer, size_t size );
uint32_t misa_aunit_iommu_translate( misa_inst *sa, uint32_t addr );
int misa_aunit_may_complete( misa_inst *sa, int sai );
void misa_aunit_bus_target_init( misa_inst *sa, const cfg_section *section );
void misa_aunit_bus_init( misa_inst *sa, const cfg_section *section );
void misa_aunit_iommu_init( misa_inst *sa, const cfg_section *section );

int misa_is_bunit_addr( misa_inst *sa, uint32_t addr );
void misa_bunit_upstream_write( misa_inst *sa, uint32_t addr, const void *buffer, size_t size );
void misa_bunit_upstream_read ( misa_inst *sa, uint32_t addr,       void *buffer, size_t size );

void misa_sram_init( misa_inst *sa, const cfg_section *section );
void misa_sram_write( misa_inst *sa, uint32_t addr, const void *buffer, size_t size );
void misa_sram_read ( misa_inst *sa, uint32_t addr,       void *buffer, size_t size );
void misa_sram_process(misa_inst *sa);

void misa_rom_init( misa_inst *sa, const cfg_section *section );
void misa_rom_write ( misa_inst *sa, uint32_t addr, const void *buffer, size_t size );
void misa_rom_read  ( misa_inst *sa, uint32_t addr,       void *buffer, size_t size );
#endif //MELOADER_EMU_H
