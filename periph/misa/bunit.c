//
// Created by pbx on 19/07/19.
//

#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include "misa/emu.h"
#include "log.h"

void misa_bunit_upstream_write( misa_inst *sa, uint32_t addr, const void *buffer, size_t size )
{
    if ( sa->bunit_user ) {
        memcpy( ( void * ) addr, buffer, size );
    } else
        log( LOG_FATAL, sa->self.name, "bunit writes not implemented" );
}

void misa_bunit_upstream_read( misa_inst *sa, uint32_t addr,        void *buffer, size_t size )
{
    if ( sa->bunit_user ) {
        memcpy( buffer, ( void * ) addr, size );
    } else
        log( LOG_FATAL, sa->self.name, "bunit read not implemented" );
}