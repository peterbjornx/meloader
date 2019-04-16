//
// Created by pbx on 15/04/19.
//
#include "meloader.h"
#include "printf.h"

#define SPI_BASE (0xF461E000);
#define SPI_SIZE (0x1000)

int spi_read(int addr, void *buffer, int count ) {
    uint32_t *buf = buffer;
    addr -= SPI_BASE;
    if ( addr < 0 || addr >= SPI_SIZE )
        return 0;
    mel_printf("[spi ] read  0x%03x count:%i\n", addr, count);
    return 1;
}

int spi_write( int addr, const void *buffer, int count ) {
    addr -= SPI_BASE;
    if ( addr < 0 || addr >= SPI_SIZE )
        return 0;
    mel_printf("[spi ] write 0x%03x count:%i\n", addr, count);
    return 1;
}


mmio_periph spi_mmio = {
        .write = spi_write,
        .read = spi_read
};


void spi_install() {
    krnl_periph_reg( &spi_mmio );
}