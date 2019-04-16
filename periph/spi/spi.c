//
// Created by pbx on 15/04/19.
//
#include "meloader.h"
#include "printf.h"
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#define SPI_BASE (0xF461E000);
#define SPI_SIZE (0x1000)

typedef struct {
    int      csme_readable;
    int      csme_writable;
    uint32_t base;
    uint32_t limit;
} spi_region_t;

typedef struct {
    uint32_t  vscc;
    uint32_t  par_data[0x200];
} spi_component_t;

spi_component_t spi_components[2];
spi_region_t spi_regions[12];
uint32_t     spi_jedec_id[0x40];
uint32_t     spi_faddr = 0;
uint32_t     spi_hsflctl = 0;
uint32_t     spi_ssfsts_ctl = 0;
uint8_t      spi_buffer[0x40];
uint32_t     spi_ptinx;
int          spi_image_file;
uint32_t     spi_opmenu[2];

uint8_t spi_read_buffer[0x40];

const char *spi_cycle_names[] = {"Read", "", "Write", "Erase4K", "Erase64K",
                                 "ReadSFDP", "ReadJEDEC", "WriteStatus",
                                 "ReadStatus","RPMCOp1","RPMCOp2","","","","",
                                 ""};

uint32_t spi_ptread();

int spi_read(int addr, void *buffer, int count ) {
    int i;
    uint32_t *buf = buffer;
    addr -= SPI_BASE;
    if ( addr < 0 || addr >= SPI_SIZE )
        return 0;
    if ( addr >= 0x10 && addr < 0x50 ) {
        addr -= 0x10;
        if ( count + addr > 0x40 ) {
            mel_printf("[spi ] bad data transfer with offset:%i count:%i\n",
                       addr, count);
            return 1;
        }
        memcpy( buffer, spi_buffer + addr, count );
        return 1;
    }
    switch ( addr ) {
        case 0x4:
            *buf = spi_hsflctl;
            break;
        case 0x50:
            *buf = 0;
            for ( i = 0; i < 8; i++ ) {
                if ( spi_regions[i].csme_readable )
                    *buf |= 1 << i;
                if ( spi_regions[i].csme_writable )
                    *buf |= 1 << (i+8);
            }
            break;
        case 0x54:
        case 0x58:
        case 0x5C:
        case 0x60:
        case 0x64:
        case 0x68:
        case 0x6C:
        case 0x70:
        case 0x74:
        case 0x78:
        case 0x7C:
        case 0x80:
            i = ( addr - 0x54) / 4;
            *buf = 0;
            *buf |= (spi_regions[i].base >> 12) & 0x7FFF;
            *buf |= (spi_regions[i].limit << 4) & 0x7FFF0000;
            mel_printf("[spi ] read CSXE_FREG%i count:%i base:%08x limit:%08x\n",
                       i, count, spi_regions[i].base, spi_regions[i].limit);
            break;
        case 0xA0:
            *buf = spi_ssfsts_ctl;
            break;
        case 0xC4:
        case 0xC8:
            i = ( addr - 0xC4) / 4;
            *buf = spi_components[i].vscc;
            break;
        case 0xCC:
            *buf = spi_ptinx;
            break;
        case 0xD0:
            *buf = spi_ptread();
                    break;
        default:
            mel_printf("[spi ] read  0x%03x count:%i\n", addr, count);
            break;
    }
    return 1;
}

uint32_t spi_ptread() {
    int comp = (spi_ptinx >> 14) & 2;
    int hord = (spi_ptinx >> 12) & 2;
    int dwi  = (spi_ptinx >> 2) & 0x1FF;
    mel_printf("[spi ] component %i par table read hord: %i dwi: %i\n",
            comp, hord, dwi);
    if ( hord == 2 )
        return spi_components[comp].par_data[dwi];
    return 0;
}

void spi_do_hwseq();
void spi_do_swseq();

int spi_write(int addr, const void *buffer, int count ) {
    int i;
    uint32_t *buf = buffer;
    addr -= SPI_BASE;
    if ( addr < 0 || addr >= SPI_SIZE )
        return 0;
    if ( addr >= 0x10 && addr < 0x50 ) {
        addr -= 0x10;
        if ( count + addr > 0x40 ) {
            mel_printf("[spi ] bad data transfer with offset:%i count:%i\n",
                       addr, count);
            return 1;
        }
        memcpy( spi_buffer + addr, buffer, count );
        return 1;
    }
    switch( addr ) {
        case 0x4:
            if ( spi_hsflctl & 0x10000 ) {
                mel_printf("[spi ] write CSXE_HSFLCTL ignored because F_GO is set\n");
                break;
            }
            spi_hsflctl &= ~(*buf & 7); /* Clearing error flags */
            spi_hsflctl &= 0x0000E1FF;
            spi_hsflctl |= *buf & 0xFFFF0000;
            break;
        case 0x8:
            spi_faddr = *buf;
            break;
        case 0xA4:
            mel_printf("[spi ] write CSXE_PREOP_OPTYPE count:%i value:%08x\n",
                    count, *buf);
            break;
        case 0xA8:
        case 0xAC:
            i = ( addr - 0xA8) / 4;
            mel_printf("[spi ] write CSXE_OPMENU%i      count:%i value:%08x\n",
                       i, count, *buf);
            spi_opmenu[i] = *buf;
            break;
        case 0x54:
        case 0x58:
        case 0x5C:
        case 0x60:
        case 0x64:
        case 0x68:
        case 0x6C:
        case 0x70:
        case 0x74:
        case 0x78:
        case 0x7C:
        case 0x80:
            i = ( addr - 0x54) / 4;
            spi_regions[i].base  = (*buf & 0x7FFF) << 12;
            spi_regions[i].limit = (*buf & 0x7FFF0000) >> 4;
            mel_printf("[spi ] write CSXE_FREG%i count:%i base:%08x limit:%08x\n",
                       i, count, spi_regions[i].base, spi_regions[i].limit);
            break;
        case 0xA0:
            if ( spi_ssfsts_ctl & 0x200 ) {
                mel_printf("[spi ] write CSXE_SSFSTS_CTL ignored because F_GO is set\n");
                break;
            }
            spi_ssfsts_ctl &= ~(*buf & 0xC); /* Clearing error flags */
            spi_ssfsts_ctl &= 0x077F7EFF;
            spi_ssfsts_ctl |= *buf & 0x077F7E00;
            break;
        case 0xCC:
            spi_ptinx = *buf;
            break;
        default:
            mel_printf("[spi ] write 0x%03x count:%i %x\n", addr, count, *buf);
            break;
    }
    spi_do_hwseq();
    spi_do_swseq();
    return 1;
}

void spi_do_hwseq() {
    int cycle, fdbc, rc, ls, i;
    if ( ~spi_hsflctl & 0x10000 )
        return;
    spi_hsflctl &= ~0x10000;
    fdbc  = ((spi_hsflctl >> 24) & 0x3F) + 1;
    cycle = (spi_hsflctl >> 17) & 0x0F;
   // mel_printf("[spi ] Executing cycle: %s from %08x count: %i\n",
   //         spi_cycle_names[cycle], spi_faddr, fdbc);
    switch ( cycle ) {
        case 0:
            ls = lseek(spi_image_file, spi_faddr, SEEK_SET);
            if (ls == -1)
                goto flash_err;
            rc = read(spi_image_file, spi_buffer, fdbc);
            if (rc != fdbc)
                goto flash_err;
            break;
        case 2:
            ls = lseek(spi_image_file, spi_faddr, SEEK_SET);
            if (ls == -1)
                goto flash_err;
            rc = read(spi_image_file, spi_read_buffer, fdbc);
            if (rc != fdbc)
                goto flash_err;
            for ( i = 0; i < fdbc; i++ )
                spi_buffer[i] &= spi_read_buffer[i]; /*Emulate flash erase req*/
            ls = lseek(spi_image_file, spi_faddr, SEEK_SET);
            if (ls == -1)
                goto flash_err;
            rc = write(spi_image_file, spi_buffer, fdbc);
            if (rc != fdbc)
                goto flash_err;
            break;
        case 6:
            memcpy( spi_buffer, spi_jedec_id, fdbc );
            break;
        default:
            break;
    }
    spi_hsflctl |= 1;
    return;
flash_err:
    mel_printf("[spi ] Flash error: %s\n", strerror(errno));
    spi_hsflctl |= 2;
}

void spi_do_swseq() {
    int cop, dbc, op;
    if ( ~spi_ssfsts_ctl & 0x200 )
        return;
    spi_ssfsts_ctl &= ~0x200;
    dbc = ((spi_ssfsts_ctl >> 16) & 0x3F) + 1;
    cop = (spi_ssfsts_ctl >> 12) & 0x07;
    op = (spi_opmenu[cop / 4] >> ((cop % 4)*8)) & 0xFF;
    mel_printf("[spi ] Executing SPI cycle: cop: %i op:%02x count: %i\n",
            cop, op, dbc);
    spi_ssfsts_ctl |= 4;
    return;
flash_err:
    mel_printf("[spi ] SPI error: %s\n", strerror(errno));
    spi_ssfsts_ctl |= 8;
}

mmio_periph spi_mmio = {
        .write = spi_write,
        .read = spi_read
};

void spi_openimg( const char *path ) {
    spi_image_file = open( path, O_RDWR );
    if ( spi_image_file == -1 ) {
        mel_printf("[spi ] Could not open image!\n");
        exit(255);
    }
    //TODO: Read Flash Descriptor
}

void spi_exampleregions() {
    spi_regions[0].base = 0x0000;
    spi_regions[0].limit = 0x1000;
    spi_regions[0].csme_readable = 1;
    spi_regions[2].base = 0x3000;
    spi_regions[2].limit = 0x6FD000;
    spi_regions[2].csme_readable = 1;
    spi_regions[2].csme_writable = 1;
    spi_jedec_id[0] = 0xEF;
    spi_jedec_id[1] = 0x40;
    spi_jedec_id[2] = 0x18;
    spi_components[0].vscc = 0xFFD820E5;
    spi_components[0].par_data[0] = 0xFFF120E5;
    /*
    w25q128fv-1: Read SFDP [0x000000] <  00 >
    53 46 44 50
    00 01 00 FF
    w25q128fv-1: 0x000008
    w25q128fv-1: Read SFDP [0x000008] <  01 >
    00 00 01 09
    80 00 00 FF
    w25q128fv-1: 0x000080
    w25q128fv-1: Read SFDP [0x000080] <  00 >
    E5 20 F1 FF
    FF FF FF 07
    44 EB 08 6B
    08 3B 42 BB
    FE FF FF FF
    FF FF 00 00
    FF FF 21 EB
    0C 20 0F 52
    10 D8 00 00*/

}

void spi_install() {
    spi_exampleregions();
    krnl_periph_reg( &spi_mmio );
}