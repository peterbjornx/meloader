//
// Created by pbx on 23/08/19.
//
#include <string.h>
#include <stdlib.h>
#include "log.h"
#include "fusectl.h"
#include "devreg.h"
#include "sideband.h"

void fusectl_dump_addr( fusectl_inst *i ) {
    int baraddr = (i->intel_address >> 17u) & 0x7u;
    int sp_modaddrmsb = (i->intel_address >> 16u) & 0x1u;
    int modaddr = (i->intel_address >> 10u) & 0x3Fu;
    int blkaddr = (i->intel_address >> 7u) & 0x7u;
    int rowaddr = (i->intel_address >> 2u) & 0x1Fu;
    log( LOG_TRACE, i->self.name, "rowaddr: %02X blkaddr: %i modaddr: %02X sp_modaddrmsb: %i baraddr: %i",
         rowaddr, blkaddr, modaddr, sp_modaddrmsb, baraddr );

}

void fusectl_dump_fsm( fusectl_inst *i ) {
    int repschmsel = (i->intel_fsm_ctrl >> 31u) & 1u;
    int fsmreset = (i->intel_fsm_ctrl >> 24u) & 1u;
    int fsmstart = (i->intel_fsm_ctrl >> 23u) & 1u;
    int hvprotect = (i->intel_fsm_ctrl >> 22u) & 1u;
    int senselvlhs = (i->intel_fsm_ctrl >> 21u) & 1u;
    int senselvlbhs = (i->intel_fsm_ctrl >> 20u) & 1u;
    int sensehizhs = (i->intel_fsm_ctrl >> 19u) & 1u;
    int favor1hs = (i->intel_fsm_ctrl >> 16u) & 7u;
    int favor0hs = (i->intel_fsm_ctrl >> 13u) & 7u;
    int senselvlhd = (i->intel_fsm_ctrl >> 12u) & 1u;
    int senselvlbhd = (i->intel_fsm_ctrl >> 11u) & 1u;
    int sensehizhd = (i->intel_fsm_ctrl >> 10u) & 1u;
    int favor1hd = (i->intel_fsm_ctrl >> 7u) & 7u;
    int favor0hd = (i->intel_fsm_ctrl >> 4u) & 7u;
    int mode_sel = (i->intel_fsm_ctrl >> 1u) & 7u;
    int gblpgmen = (i->intel_fsm_ctrl >> 0u) & 1u;
    log( LOG_TRACE, i->self.name, "gblpgmen: %i mode_sel: %i hvprotect: %i fsmstart: %i fsmreset: %i repschmsel: %i",
            gblpgmen, mode_sel, hvprotect, fsmstart, fsmreset, repschmsel );
    log( LOG_TRACE, i->self.name, "favor0hd: %i favor1hd: %i sensehizhd: %i senselvlbhd: %i senselvlhd: %i",
         favor0hd, favor1hd, sensehizhd, senselvlbhd, senselvlhd );
    log( LOG_TRACE, i->self.name, "favor0hs: %i favor1hs: %i sensehizhs: %i senselvlbhs: %i senselvlhs: %i",
         favor0hs, favor1hs, sensehizhs, senselvlbhs, senselvlhs );

}

void fusectl_process( fusectl_inst *i ) {
    if ( i->control & 0x20000u ) {
        log( LOG_ERROR, i->self.name, "Requested command 0x20000 with control word %08X", i->control );
        i->control &= ~(0x20000u | 0x90u);
        i->control |= 0x90u;
    }
    if ( i->control & 0x40000u ) {
        log( LOG_ERROR, i->self.name, "Requested command 0x40000 with control word %08X", i->control );
        i->control &= ~(0x40000u | 0x90u);
        i->control = 0x90u;
    }
    if ( i->intel_debug_status & ( 1u << 31u ) )
        i->intel_debug_status = 0;
    if ( i->intel_fsm_ctrl & 1u << 23u ) {
        i->intel_fsm_ctrl &= ~(1u<<23u);
        log( LOG_ERROR, i->self.name, "Requested FSM start with control word %08X", i->intel_fsm_ctrl );
        fusectl_dump_fsm( i );
        fusectl_dump_addr( i );
        i->intel_debug_status |= (1u<<1u);//sense_done
    }
    if ( i->intel_fsm_ctrl & 1u << 24u ) {
        i->intel_fsm_ctrl &= ~(1u<<24u);
        log( LOG_ERROR, i->self.name, "Requested FSM stop with control word %08X", i->intel_fsm_ctrl );
    }
}

int fusectl_read( sideband_dev *dev, int bar, int op, int offset, void *buffer, int count, int sai ) {
    fusectl_inst *i = dev->device->impl;
    if (op != 6) {
        log(LOG_ERROR, dev->device->name, "Invalid sideband opcode %i in read", op);
        return -2;
    }
    log(LOG_INFO, dev->device->name, "BAR%i offset:%08x count:%i", bar, offset, count);
    if (bar == 4) {
        if (count == 4 && offset == 0x0000) {
            *(uint32_t *) buffer = i->control;
            log(LOG_TRACE, dev->device->name, "Read control register: %08X", i->control);
        } else if (count == 4 && offset == 0x0004) {
            *(uint32_t *) buffer = i->intel_global_ctrl;
            log(LOG_TRACE, dev->device->name, "Read intel_global_ctrl register: %08X", *(uint32_t *) buffer);
        } else if (count == 4 && offset == 0x0008) {
            *(uint32_t *) buffer = i->intel_kar;
            log(LOG_DEBUG, dev->device->name, "Read intel_kar register: %08X", *(uint32_t *) buffer);
        } else if (count == 4 && offset == 0x000C) {
            *(uint32_t *) buffer = i->intel_address;
            log(LOG_TRACE, dev->device->name, "Read intel_address register: %08X", *(uint32_t *) buffer);
        } else if (count == 4 && offset == 0x0010) {
            *(uint32_t *) buffer = i->intel_size;
            log(LOG_DEBUG, dev->device->name, "Read intel_size register: %08X", *(uint32_t *) buffer);
        } else if (count == 4 && offset == 0x0014) {
            *(uint32_t *) buffer = i->intel_fsm_ctrl;
            log(LOG_TRACE, dev->device->name, "Read intel_fsm_ctrl register: %08X", *(uint32_t *) buffer);
        } else if (count == 4 && offset == 0x001C) {
            *(uint32_t *) buffer = i->intel_hd_counter;
            log(LOG_TRACE, dev->device->name, "Read intel_hd_counter register: %08X", *(uint32_t *) buffer);
        } else if (count == 4 && offset == 0x80) {
            *(uint32_t *) buffer = i->intel_debug_status;
            log(LOG_TRACE, dev->device->name, "Read intel_debug_status register: %08X", *(uint32_t *) buffer);
        } else if (count == 4 && offset == 0x1020) {
            *(uint32_t *) buffer = i->unknown_1020;
            log(LOG_DEBUG, dev->device->name, "Read unknown_1020 register: %08X", *(uint32_t *) buffer);
        } else if (count == 4 && offset == 0x1024) {
            *(uint32_t *) buffer = i->unknown_1024;
            log(LOG_DEBUG, dev->device->name, "Read unknown_1024 register: %08X", *(uint32_t *) buffer);
        } else {
            log(LOG_ERROR, dev->device->name, "Try read control offset %X size %i", offset, count);
            return -2;
        };
        return i->sai;
    } else if ( bar == 1 ) {
        if (count == 4 && offset >= 0x400 && offset <= 0x7FF && (offset & 3u) == 0 ) {
            int blkaddr = ( offset >> 7u ) & 0x7u;
            int wordaddr = ( offset >> 2u ) & 0x1Fu;
            *(uint32_t *) buffer = i->unknown_1024;
            log(LOG_TRACE, dev->device->name, "Read BAR 1 op  : blkaddr: %i wordaddr: %02X size: %i ", blkaddr, wordaddr, count );
        } else if (count == 4 && offset >= 0x000 && offset <= 0x3FF && (offset & 3u) == 0 ) {
            int blkaddr = ( offset >> 7u ) & 0x7u;
            int wordaddr = ( offset >> 2u ) & 0x1Fu;
            *(uint32_t *) buffer = i->unknown_1024;
            log(LOG_TRACE, dev->device->name, "Read BAR 1 base: blkaddr: %i wordaddr: %02X size: %i ", blkaddr, wordaddr, count );
        } else {
            log(LOG_ERROR, dev->device->name, "Try read BAR 1 offset %X size %i", offset, count);
            return -2;
        }
        return i->sai;

    } else {
        log( LOG_ERROR, dev->device->name, "Invalid BAR %i in read", bar );
    }
    return -2;
}

int fusectl_write( sideband_dev *dev, int bar, int op, int offset, const void *buffer, int count, int sai ) {
    fusectl_inst *i = dev->device->impl;
    if ( op != 7 ) {
        log( LOG_ERROR, dev->device->name, "Invalid sideband opcode %i in write", op );
        return -2;
    }
    log(LOG_TRACE, dev->device->name, "BAR%i offset:%08x count:%i data:%08x", bar, offset, count, *(uint32_t*)buffer);
    if ( bar == 4 ) {
        if ( count == 4 && offset == 0x0000 ) {
            i->control = *(uint32_t *) buffer;
            log(LOG_TRACE, dev->device->name, "Write control register: %08X", i->control);
        } else if ( count == 4 && offset == 0x0004 ) {
            i->intel_global_ctrl = *(uint32_t *) buffer;
            log(LOG_TRACE, dev->device->name, "Write intel_global_ctrl register: %08X", *(uint32_t *) buffer);
        } else if ( count == 4 && offset == 0x0008 ) {
            i->intel_kar = *(uint32_t *) buffer;
            log(LOG_DEBUG, dev->device->name, "Write intel_kar register: %08X", *(uint32_t *) buffer);
        } else if ( count == 4 && offset == 0x000C ) {
            i->intel_address = *(uint32_t *) buffer;
            log(LOG_TRACE, dev->device->name, "Write intel_address register: %08X", *(uint32_t *) buffer);
        } else if ( count == 4 && offset == 0x0010 ) {
            i->intel_size = *(uint32_t *) buffer;
            log(LOG_DEBUG, dev->device->name, "Write intel_size register: %08X", *(uint32_t *) buffer);
        } else if ( count == 4 && offset == 0x0014 ) {
            i->intel_fsm_ctrl = *(uint32_t *) buffer;
            log(LOG_TRACE, dev->device->name, "Write intel_fsm_ctrl register: %08X", *(uint32_t *) buffer);
        } else if ( count == 4 && offset == 0x001C ) {
            i->intel_hd_counter = *(uint32_t *) buffer;
            log(LOG_TRACE, dev->device->name, "Write intel_hd_counter register: %08X", *(uint32_t *) buffer);
        } else if ( count == 4 && offset == 0x80 ) {
            i->intel_debug_status = *(uint32_t *) buffer;
            log(LOG_TRACE, dev->device->name, "Write intel_debug_status register: %08X", *(uint32_t *) buffer);
        } else if ( count == 4 && offset == 0x1020 ) {
            i->unknown_1020 = *(uint32_t *) buffer;
            log(LOG_TRACE, dev->device->name, "Write unknown_1020 register: %08X", *(uint32_t *) buffer);
        } else if ( count == 4 && offset == 0x1024 ) {
            i->unknown_1024 = *(uint32_t *) buffer;
            log(LOG_TRACE, dev->device->name, "Write unknown_1024 register: %08X", *(uint32_t *) buffer);
        } else {
            log(LOG_ERROR, dev->device->name, "Try write control offset %X size %i", offset, count);
            return -2;
        }
        fusectl_process( i );
        return i->sai;

    }
    return -2;

}

device_instance * fusectl_spawn(const cfg_file *file, const cfg_section *section) {
    int s;
    fusectl_inst *i = malloc( sizeof(fusectl_inst) );
    logassert( i != NULL, section->name, "Could not allocate instance structure" );
    memset( i, 0, sizeof(fusectl_inst) );
    i->self.name = section->name;
    i->self.impl = i;

    s = cfg_find_int32(section, "endpoint", (uint32_t *) &i->sb.endpoint);
    logassert( s >= 0, section->name, "No sideband endpoint specified" );

    s = cfg_find_int32(section, "sai", (uint32_t *) &i->sai);
    logassert( s >= 0, section->name, "No SAI specified" );

    i->sb.device = &i->self;
    i->sb.read = fusectl_read;
    i->sb.write = fusectl_write;

    sb_register( &i->sb );
    return &i->self;
}

device_type fusectl_type = {
        .name = "fusectl",
        .spawn = fusectl_spawn
};

static __attribute__((constructor)) void register_fusectl() {
    device_type_register( &fusectl_type );
}
