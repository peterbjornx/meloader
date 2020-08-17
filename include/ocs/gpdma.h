#ifndef MELOADER_GPDMA_H
#define MELOADER_GPDMA_H
#include <stdint.h>

#define GPDMA_REG_SRC_ADDR  (0x400)
#define GPDMA_REG_DST_ADDR  (0x404)
#define GPDMA_REG_SRC_SIZE  (0x408)
#define GPDMA_REG_DST_SIZE  (0x40C)
#define GPDMA_REG_CONTROL   (0x410)
#define GPDMA_REG_RES       (0x414)
#define GPDMA_REG_STATUS    (0x428)


typedef struct {
    uint32_t src_addr;
    uint32_t src_size;
    uint32_t dst_addr;
    uint32_t dst_size;
    uint32_t control;
    uint32_t status;
    void (*bus_read) ( void *i, uint32_t addr, void *data, size_t size );
    void (*bus_write)( void *i, uint32_t addr, const void *data, size_t size );
    void (*int_write)( void *i, const void *data, uint32_t size );
    void (*int_read) ( void *i, void *data, uint32_t size );
    void *impl;
    void *bus_impl;
} gpdma_state;

void gpdma_init( gpdma_state *state );
int gpdma_read( gpdma_state *state, int addr, void *buffer, int count );
int gpdma_write( gpdma_state *state, int addr, const void *buffer, int count );
#endif //MELOADER_GPDMA_H
