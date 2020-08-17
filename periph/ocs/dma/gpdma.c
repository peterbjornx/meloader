#include "log.h"
#include "user/meloader.h"
#include <stdint.h>
#include "ocs/gpdma.h"

int gpdma_read( gpdma_state *state, int addr, void *buffer, int count ) {
    uint32_t *buf = buffer;
    if ( addr < 0x400 || addr >= 0x500 )
        return 0;
    if ( count != 4 || (addr & 3) ) {
        log(LOG_ERROR, "gpdma",
                "read misaligned 0x%03x count:%i", addr, count);
        return 1;
    }
    if ( addr == GPDMA_REG_SRC_ADDR )
        *buf = state->src_addr;
    else if ( addr == GPDMA_REG_DST_ADDR )
        *buf = state->dst_addr;
    else if ( addr == GPDMA_REG_SRC_SIZE )
        *buf = state->src_size;
    else if ( addr == GPDMA_REG_DST_SIZE )
        *buf = state->dst_size;
    else if ( addr == GPDMA_REG_RES )
        *buf = 0;
    else if ( addr == GPDMA_REG_CONTROL ) {
        *buf = state->control;
        log(LOG_TRACE, "gpdma", "read control: 0x%08x", *buf);
    } else if ( addr == GPDMA_REG_STATUS ) {
        *buf = state->status;
        log(LOG_TRACE, "gpdma", "read status : 0x%08x", *buf);
    } else
        log(LOG_ERROR, "gpdma", "read  0x%03x count:%i", addr, count);
    return 1;

}

uint8_t gpdma_buffer[64];

void gpdma_run_transaction( gpdma_state *state ) {
    int turnsize, count, pos;
    if ( ~state->control & (1 << 31) )
        return;
    state->control &= ~(1 << 31);
    state->status = 0xc000;
    count = state->src_size;
    if ( state->dst_size > count )
        count = state->dst_size;
    pos = 0;
    log(LOG_DEBUG, "gpdma", "Run transaction src:0x%08x dst:0x%08x sz:0x%08x ctrl:%08X",
            state->src_addr, state->dst_addr, count, state->control);
    while ( count ) {
        turnsize = count;
        if (turnsize > sizeof gpdma_buffer)
            turnsize = sizeof gpdma_buffer;
        if ( state->src_addr ) {
            state->bus_read(state->bus_impl, state->src_addr + pos, gpdma_buffer, (size_t) turnsize);
        } else {
            state->int_read(state->impl, gpdma_buffer, (uint32_t) turnsize);
        }
        if ( state->dst_addr ) {
            state->bus_write(state->bus_impl, state->dst_addr + pos, gpdma_buffer, (size_t) turnsize);
        }
        if ( ~state->control & 0x40000000 ){
            state->int_write(state->impl, gpdma_buffer, (uint32_t) turnsize);
        }
        pos += turnsize;
        count -= turnsize;
    }

}

int gpdma_write( gpdma_state *state, int addr, const void *buffer, int count ) {
    const uint32_t *buf = buffer;
    if ( addr < 0x400 || addr >= 0x500 )
        return 0;
    if ( count != 4 || (addr & 3) ) {
        log(LOG_ERROR, "gpdma",
            "write misaligned 0x%03x count:%i", addr, count);
        return 1;
    }
    if ( addr == GPDMA_REG_SRC_ADDR )
        state->src_addr = *buf;
    else if ( addr == GPDMA_REG_DST_ADDR )
        state->dst_addr = *buf;
    else if ( addr == GPDMA_REG_SRC_SIZE )
        state->src_size = *buf;
    else if ( addr == GPDMA_REG_DST_SIZE )
        state->dst_size = *buf;
    else if ( addr == GPDMA_REG_RES ) {
        log(LOG_DEBUG, "gpdma", "write RES = %08X", addr, *buf);}
    else if ( addr == GPDMA_REG_CONTROL ) {
        state->control = *buf;
        //mel_printf("[gdma] write control: 0x%08x", *buf);
    } else if ( addr == GPDMA_REG_STATUS )
        state->status = *buf;
    else
        log(LOG_ERROR, "gpdma", "write 0x%03x count:%i", addr, count);
    gpdma_run_transaction( state );
    return 1;
}

void gpdma_init( gpdma_state *buffer ) {
    buffer->control = 0;
    buffer->status = 0;
}