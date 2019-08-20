//
// Created by pbx on 08/08/19.
//

#ifndef MELOADER_FSB_H
#define MELOADER_FSB_H

#include <stdint.h>
#include <stddef.h>
#include "devreg.h"
;
typedef struct {
    device_instance *cpu;
    device_instance *sa;
    /** Perform a memory write downstream to the system agent */
    void (*sa_mem_write    ) ( device_instance *sa, uint32_t addr, const void *buffer, size_t size );
    /** Perform a memory read downstream to the system agent */
    void (*sa_mem_read     ) ( device_instance *sa, uint32_t addr,       void *buffer, size_t size );
    /** Perform an IO write downstream to the system agent */
    void (*sa_io_write     ) ( device_instance *sa, uint32_t addr, const void *buffer, size_t size );
    /** Perform an IO read downstream to the system agent */
    void (*sa_io_read      ) ( device_instance *sa, uint32_t addr,       void *buffer, size_t size );
    /** Perform a memory write upstream to the CPU */
    void (*mia_mem_write   ) ( device_instance *mia, uint32_t addr, const void *buffer, size_t size );
    /** Perform a memory read upstream to the CPU */
    void (*mia_mem_read    ) ( device_instance *mia, uint32_t addr,       void *buffer, size_t size );
    /** Update CPU legacy wires */
    void (*mia_legacy_wire ) ( device_instance *mia, int w_int, int w_nmi );

} mia_fsb;

#endif //MELOADER_FSB_H
