//
// Created by pbx on 16/11/19.
//

#ifndef MELOADER_HWIF_H
#define MELOADER_HWIF_H

#include <stdint.h>

#define HWIF_PACKET_MM_WRITE (0x00)
#define HWIF_PACKET_MM_READ  (0x01)
#define HWIF_PACKET_SB_WRITE (0x02)
#define HWIF_PACKET_SB_READ  (0x03)

typedef struct __attribute__((packed)) {
    int endpt;
    int rd_op;
    int wr_op;
    int func;
    int bar;
    int sba28;
    int rs;
    int sba29;
} hwif_sbaddr;

void hwif_connect( const char *host, int port );

int hwif_sb_write(
    hwif_sbaddr addr,
    uint32_t  offset,
    const void *buffer,
    int size );
int hwif_sb_read(
    hwif_sbaddr addr,
    uint32_t  offset,
    void *buffer,
    int size );
int hwif_mm_write(
    uint32_t    addr,
    const void *buffer,
    int size );
int hwif_mm_read(
    uint32_t    addr,
    void *buffer,
    int size );
void hwif_disconnect();
#endif //MELOADER_HWIF_H
