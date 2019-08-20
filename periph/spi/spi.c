//
// Created by pbx on 15/04/19.
//
#include "user/meloader.h"
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include "fastspi.h"
#include "log.h"

const char *spi_cycle_names[] = {"Read", "", "Write", "Erase4K", "Erase64K",
                                 "ReadSFDP", "ReadJEDEC", "WriteStatus",
                                 "ReadStatus","RPMCOp1","RPMCOp2","","","","",
                                 ""};

uint32_t spi_ptread( fastspi_inst *spi );

int spi_read( fastspi_inst *spi, int addr, void *buffer, int count ) {
    int i;
    uint32_t *buf = buffer;
    if ( addr >= 0x10 && addr < 0x50 ) {
        addr -= 0x10;
        if ( count + addr > 0x40 ) {
            log(LOG_ERROR, spi->self.name, "bad data transfer with offset:%i count:%i",
                       addr, count);
            return 1;
        }
        memcpy( buffer, spi->spi_buffer + addr, count );
        return 1;
    }
    switch ( addr ) {
        case 0x4:
            *buf = spi->spi_hsflctl;
            break;
        case 0x50:
            *buf = 0;
            for ( i = 0; i < 8; i++ ) {
                if ( spi->spi_regions[i].csme_readable )
                    *buf |= 1 << i;
                if ( spi->spi_regions[i].csme_writable )
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
            *buf |= (spi->spi_regions[i].base >> 12) & 0x7FFF;
            *buf |= (spi->spi_regions[i].limit << 4) & 0x7FFF0000;
            log(LOG_TRACE, spi->self.name, "read CSXE_FREG%i count:%i base:%08x limit:%08x",
                       i, count, spi->spi_regions[i].base, spi->spi_regions[i].limit);
            break;
        case 0xA0:
            *buf = spi->spi_ssfsts_ctl;
            break;
        case 0xC4:
        case 0xC8:
            i = ( addr - 0xC4) / 4;
            *buf = spi->spi_components[i].vscc;
            break;
        case 0xCC:
            *buf = spi->spi_ptinx;
            break;
        case 0xD0:
            *buf = spi_ptread(spi);
                    break;
        default:
            log(LOG_ERROR, spi->self.name, "unknown reg read  0x%03x count:%i", addr, count);
            break;
    }
    return 1;
}

uint32_t spi_ptread( fastspi_inst *spi ) {
    int comp = (spi->spi_ptinx >> 14) & 2;
    int hord = (spi->spi_ptinx >> 12) & 2;
    int dwi  = (spi->spi_ptinx >> 2) & 0x1FF;
    log(LOG_TRACE, spi->self.name, "component %i par table read hord: %i dwi: %i",
            comp, hord, dwi);
    if ( hord == 2 )
        return spi->spi_components[comp].par_data[dwi];
    return 0;
}

void spi_do_hwseq(fastspi_inst *spi);
void spi_do_swseq(fastspi_inst *spi);

int spi_write( fastspi_inst *spi, int addr, const void *buffer, int count ) {
    int i;
    uint32_t *buf = buffer;
    if ( addr >= 0x10 && addr < 0x50 ) {
        addr -= 0x10;
        if ( count + addr > 0x40 ) {
            log(LOG_ERROR, spi->self.name, "bad data transfer with offset:%i count:%i",
                       addr, count);
            return 1;
        }
        memcpy( spi->spi_buffer + addr, buffer, count );
        return 1;
    }
    switch( addr ) {
        case 0x4:
            if ( spi->spi_hsflctl & 0x10000 ) {
                log(LOG_WARN, spi->self.name, "write CSXE_HSFLCTL ignored because F_GO is set");
                break;
            }
            log(LOG_TRACE, spi->self.name, "write CSXE_HSFLCTL count:%i value:%08x",
                count, *buf);
            spi->spi_hsflctl &= ~(*buf & 7); /* Clearing error flags */
            spi->spi_hsflctl &= 0x0000E1FF;
            spi->spi_hsflctl |= *buf & 0xFFFF0000;
            break;
        case 0x8:
            spi->spi_faddr = *buf;
            break;
        case 0xA4:
            log(LOG_TRACE, spi->self.name, "write CSXE_PREOP_OPTYPE count:%i value:%08x",
                    count, *buf);
            break;
        case 0xA8:
        case 0xAC:
            i = ( addr - 0xA8) / 4;
            log(LOG_TRACE, spi->self.name, "write CSXE_OPMENU%i      count:%i value:%08x",
                       i, count, *buf);
            spi->spi_opmenu[i] = *buf;
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
            spi->spi_regions[i].base  = (*buf & 0x7FFF) << 12;
            spi->spi_regions[i].limit = (*buf & 0x7FFF0000) >> 4;
            log(LOG_TRACE, spi->self.name, "write CSXE_FREG%i count:%i base:%08x limit:%08x",
                       i, count, spi->spi_regions[i].base, spi->spi_regions[i].limit);
            break;
        case 0xA0:
            if ( spi->spi_ssfsts_ctl & 0x200 ) {
                log(LOG_WARN, spi->self.name, "write CSXE_SSFSTS_CTL ignored because F_GO is set");
                break;
            }
            spi->spi_ssfsts_ctl &= ~(*buf & 0xC); /* Clearing error flags */
            spi->spi_ssfsts_ctl &= 0x077F7EFF;
            spi->spi_ssfsts_ctl |= *buf & 0x077F7E00;
            break;
        case 0xCC:
            spi->spi_ptinx = *buf;
            break;
        default:
            log(LOG_TRACE, spi->self.name, "write 0x%03x count:%i %x", addr, count, *buf);
            break;
    }
    spi_do_hwseq( spi );
    spi_do_swseq( spi );
    return 1;
}

void spi_do_hwseq( fastspi_inst *spi ) {
    int cycle, fdbc, rc, ls, i;
    if ( ~spi->spi_hsflctl & 0x10000 )
        return;
    spi->spi_hsflctl &= ~0x10000;
    fdbc  = ((spi->spi_hsflctl >> 24) & 0x3F) + 1;
    cycle = (spi->spi_hsflctl >> 17) & 0x0F;
    log(LOG_TRACE, spi->self.name, "Executing cycle: %s from %08x count: %i",
             spi_cycle_names[cycle], spi->spi_faddr, fdbc);
    switch ( cycle ) {
        case 0:
            ls = lseek(spi->spi_image_file, spi->spi_faddr, SEEK_SET);
            if (ls == -1)
                goto flash_err;
            rc = read(spi->spi_image_file, spi->spi_buffer, fdbc);
            if (rc != fdbc)
                goto flash_err;
            break;
        case 2:
            ls = lseek(spi->spi_image_file, spi->spi_faddr, SEEK_SET);
            if (ls == -1)
                goto flash_err;
            rc = read(spi->spi_image_file, spi->spi_read_buffer, fdbc);
            if (rc != fdbc)
                goto flash_err;
            for ( i = 0; i < fdbc; i++ )
                spi->spi_buffer[i] &= spi->spi_read_buffer[i]; /*Emulate flash erase req*/
            ls = lseek(spi->spi_image_file, spi->spi_faddr, SEEK_SET);
            if (ls == -1)
                goto flash_err;
            rc = write(spi->spi_image_file, spi->spi_buffer, fdbc);
            if (rc != fdbc)
                goto flash_err;
            break;
        case 6:
            memcpy( spi->spi_buffer, spi->spi_jedec_id, fdbc );
            break;
        default:
            break;
    }
    spi->spi_hsflctl |= 1;
    return;
flash_err:
    log(LOG_ERROR, spi->self.name, "Flash error: %s", strerror(errno));
    spi->spi_hsflctl |= 2;
}

void spi_do_swseq( fastspi_inst *spi ) {
    int cop, dbc, op;
    if ( ~spi->spi_ssfsts_ctl & 0x200 )
        return;
    spi->spi_ssfsts_ctl &= ~0x200;
    dbc = ((spi->spi_ssfsts_ctl >> 16) & 0x3F) + 1;
    cop = (spi->spi_ssfsts_ctl >> 12) & 0x07;
    op = (spi->spi_opmenu[cop / 4] >> ((cop % 4)*8)) & 0xFF;
    log(LOG_TRACE, spi->self.name, "Executing SPI cycle: cop: %i op:%02x count: %i",
            cop, op, dbc);
    spi->spi_ssfsts_ctl |= 4;
    return;
flash_err:
    log(LOG_ERROR, spi->self.name, "SPI error: %s", strerror(errno));
    spi->spi_ssfsts_ctl |= 8;
}

void spi_openimg( fastspi_inst *spi, const char *path ) {
    spi->spi_image_file = open( path, O_RDWR );
    if ( spi->spi_image_file == -1 ) {
        log(LOG_ERROR, spi->self.name, "Could not open image %s!", path);
        exit(255);
    }
    //TODO: Read Flash Descriptor
}