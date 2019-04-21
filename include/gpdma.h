#ifndef MELOADER_GPDMA_H
#define MELOADER_GPDMA_H
#include <stdint.h>

#define GPDMA_REG_SRC_ADDR  (0x400)
#define GPDMA_REG_DST_ADDR  (0x404)
#define GPDMA_REG_SRC_SIZE  (0x408)
#define GPDMA_REG_DST_SIZE  (0x40C)
#define GPDMA_REG_CONTROL   (0x410)
#define GPDMA_REG_STATUS    (0x428)


typedef struct {
    uint32_t src_addr;
    uint32_t src_size;
    uint32_t dst_addr;
    uint32_t dst_size;
    uint32_t control;
    uint32_t status;
    void (*int_write)( void *data, uint32_t size );
    void (*int_read) ( void *data, uint32_t size );
} gpdma_state;

void gpdma_init( gpdma_state *state );
int gpdma_read( gpdma_state *state, int addr, void *buffer, int count );
int gpdma_write( gpdma_state *state, int addr, const void *buffer, int count );
#endif //MELOADER_GPDMA_H
