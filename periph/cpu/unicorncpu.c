//
// Created by pbx on 02/09/19.
//
#include "log.h"
#include "fsb.h"
#include "cpu/unicorncpu.h"
#include <string.h>
#include <stdlib.h>

void libia32cpu_legacywire( device_instance *inst, int m_int, int m_nmi) {
    libia32cpu_inst *i = inst->impl;
    static int v_int = 0;
    static int v_nmi = 0;
    if ( v_int != m_int ){
        v_int = m_int;
        if ( v_int )
            libia32_corecplx_raise_intr( i->engine );
        else
            libia32_corecplx_clear_intr( i->engine );
    }
    if ( v_nmi != m_nmi ){
        v_nmi = m_nmi;
        if ( v_int )
            libia32_corecplx_deliver_nmi( i->engine );
    }
}

void libia32cpu_cpuread(device_instance *inst, uint32_t m_int, void *m_nmi, size_t i) {
    log(LOG_ERROR, inst->name, "mia_mem_read is not yet implemented");
}

void libia32cpu_cpuwrite(device_instance *inst, uint32_t m_int, const void *m_nmi, size_t i) {
    log(LOG_ERROR, inst->name, "mia_mem_write is not yet implemented");
}

static void port_in(
        libia32_corecplx *context,
        uint32_t port,
        void *buffer,
        size_t size ) {
    libia32cpu_inst *i = context->user_context;

    logassert( size >= 1 && size <= 4, i->self.name, "Bad port read addr: %x size: %i", port, size );
    i->fsb.sa_io_read( i->fsb.sa, port, buffer, size );
}

static void port_out(
        libia32_corecplx *context,
        uint32_t port,
        const void *buffer,
        size_t size ) {
    libia32cpu_inst *i = context->user_context;

    logassert( size >= 1 && size <= 4, i->self.name, "Bad port write addr: %x size: %i", port, size );
    i->fsb.sa_io_write( i->fsb.sa, port, buffer, size );
}

static void mem_read(
        libia32_corecplx *context,
        uint64_t port,
        void *buffer,
        size_t size ) {
    libia32cpu_inst *i = context->user_context;


//    log( LOG_TRACE, i->self.name, "Mem read: %x size: %i %04X", port, size, *(uint32_t) buffer );
    i->fsb.sa_mem_read( i->fsb.sa, port, buffer, size );
 //  log( LOG_TRACE, i->self.name, "Mem read: %x size: %i %08X", port, size, *(uint32_t*) buffer );

}

static void mem_write(
        libia32_corecplx *context,
        uint64_t port,
        const void *buffer,
        size_t size ) {
    libia32cpu_inst *i = context->user_context;

  //  log( LOG_TRACE, i->self.name, "Mem write: %x size: %i", port, size );
    i->fsb.sa_mem_write( i->fsb.sa, port, buffer, size );
}

static void cpu_log (
        libia32_corecplx *context,int level, const char *format, va_list list ) {
    libia32cpu_inst *i = context->user_context;
    vlog( level, i->self.name, format, list );
}
static libia32_callbacks callbacks = {
    .io_read   = port_in,
    .io_write  = port_out,
    .mem_read  = mem_read,
    .mem_write = mem_write,
    .log = cpu_log
};

static void process( device_instance *inst ) {
    libia32cpu_inst *i = inst->impl;
    libia32_corecplx_run_slice( i->engine );
}

device_instance * libia32cpu_spawn(const cfg_file *file, const cfg_section *section) {
    libia32cpu_inst *i = malloc( sizeof(libia32cpu_inst) );
    int s;
    libia32_params params = {};
    if ( !i ) {
        log( LOG_FATAL, section->name, "Could not allocate CPU instance structure" );
        exit(EXIT_FAILURE);
    }
    memset( i, 0, sizeof(libia32cpu_inst) );
    params.ips = 42;
    i->self.impl = i;
    i->self.name = section->name;
    i->fsb.cpu = &i->self;
    i->fsb.mia_legacy_wire = libia32cpu_legacywire;
    i->fsb.mia_mem_read    = libia32cpu_cpuread;
    i->fsb.mia_mem_write   = libia32cpu_cpuwrite;
    i->engine = libia32_corecplx_create( &params, &callbacks );
    logassert( i->engine != NULL, section->name, "Error creating core complex" );
    i->engine->user_context = i;
    i->self.process = process;
    device_register( &i->self );
    libia32_corecplx_reset( i->engine, 0 );
    return &i->self;
}

device_type libia32cpu_type = {
        .name = "libia32cpu",
        .spawn = libia32cpu_spawn
};

static __attribute__((constructor)) void register_libia32cpu() {
    device_type_register( &libia32cpu_type );
}