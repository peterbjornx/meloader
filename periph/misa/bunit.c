//
// Created by pbx on 19/07/19.
//

#include <stddef.h>
#include <stdint.h>
#include "misa/emu.h"
#include "log.h"

void misa_bunit_upstream_write( misa_inst *sa, uint32_t addr, const void *buffer, size_t size )
{
    log( LOG_FATAL, sa->self.name, "bunit writes not implemented" );
}

void misa_bunit_upstream_read( misa_inst *sa, uint32_t addr,        void *buffer, size_t size )
{
    log( LOG_FATAL, sa->self.name, "bunit reads not implemented" );
}