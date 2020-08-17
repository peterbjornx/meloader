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

uint8_t spi_cacheread_hdblk( fastspi_inst *spi, int pos ) {
    uint32_t cache_start = pos & ~(SPI_HDBLK_CACHE_SIZE - 1u);
    uint32_t cache_index = pos & (SPI_HDBLK_CACHE_SIZE - 1u);
    if ( cache_start == spi->cached_hdblk_base )
        return spi->cached_hdblk[ cache_index ];

    spi->cached_hdblk_base = cache_start;

    spi_read_csme( spi, cache_start,
                   SPI_HDBLK_CACHE_SIZE,
                   spi->cached_hdblk ); //TODO: Handle errors

    return spi->cached_hdblk[ cache_index ];
}

void spi_cache_hdlut( fastspi_inst *spi ) {
    uint32_t csxefctrl = *(uint32_t *)(spi->func.func.config.bytes + 0x80);
    uint32_t hd_lutb = csxefctrl & 0x07FFFFC0u;
    uint32_t hd_compoff = spi->spi_hdcompoff;
    uint32_t guessed_size;

    guessed_size = hd_compoff - hd_lutb;

    if ( hd_compoff <= hd_lutb ) {
        /* Compressed data starts before header */

        /* We can NOT guess the LUT size in this situation */
        log( LOG_WARN, spi->self.name, "huffman LUT guess failed, HDLUTB: %08X"
                                       " HDCOMPOFF: %08X",
                hd_lutb, hd_compoff );
        guessed_size = SPI_HDLUT_CACHE_SIZE;
    } else if ( guessed_size > SPI_HDLUT_CACHE_SIZE ) {
        log( LOG_WARN, spi->self.name,
             "huffman LUT size larger than cache: 0x%08",
             guessed_size );
        guessed_size = SPI_HDLUT_CACHE_SIZE;
    }

    /* Determined size, now proceed to load HDLUT to cache */

    guessed_size /= sizeof( uint32_t );

    /* Check if the right value is already cached */
    if ( spi->cached_hdlut_base == hd_lutb &&
         spi->cached_hdlut_size >= guessed_size )
        return;

    spi->cached_hdlut_base = hd_lutb;
    spi->cached_hdlut_size = guessed_size;

    spi_read_csme( spi, hd_lutb,
            guessed_size * sizeof( uint32_t),
            spi->cached_hdlut ); //TODO: Handle errors

}

#define SPI_CSXE_FCTRL_HDEN (0x10u)
#define SPI_CSXE_FCTRL_PAGESZ (0x10u)

extern huff_map_t huff11_map[0x8000][2];
int trig=0;
void fakepage(char *out) {
    memset(out,0x0,0x1000);
    memcpy(out + 0x606, "\xB8\x42\x42\x37\x13\xA3\x18\x00\x08\xF0\xF4\xEB\xFD",14);
}

int spi_huffman_read_page( fastspi_inst *spi, int page, void *page_base ) {
    uint32_t hd_compoff = spi->spi_hdcompoff;
    uint32_t csxefctrl = *(uint32_t *)(spi->func.func.config.bytes + 0x80);
    uint32_t hd_limit  = *(uint32_t *)(spi->func.func.config.bytes + 0x84) & ~0xFFFu;
    uint32_t blk_start, blk_flags;
    uint32_t page_sz = 1024;

    /* Determine page size */
    if ( csxefctrl & SPI_CSXE_FCTRL_PAGESZ )
        page_sz = 4096;
    if ( trig && page == 0x9 && hd_compoff == 0x708C) {
        if ( trig != 2 ) {
            log(LOG_WARN,spi->self.name,"Hot-replacing block with shellcode: addr:%08X, page:%X!\n",hd_compoff,page);
            trig = 2;
        }
        fakepage(page_base);
        return 1;
    }
    //log(LOG_TRACE,spi->self.name,"huff addr:%08X, page:%X",hd_compoff,page);

    blk_start = spi->cached_hdlut[ page ] & 0x1FFFFFFu;
    blk_flags = (spi->cached_hdlut[ page ] >> 30u) & 0x2u;
    uint32_t bit_buffer = 0, available_bits = 0;
    uint32_t cmprpos = blk_start + hd_compoff;

    for ( int i = 0; i < page_sz;  ) {
        while ( available_bits <= 24  /* check for limit ? */ ) {
            bit_buffer |= spi_cacheread_hdblk( spi, cmprpos++ ) << ( 24u - available_bits );
            available_bits += 8;
        }

        uint32_t codeword = bit_buffer >> ( 32u - 15u );
        huff_map_t *map = &huff11_map[codeword & 0x7FFFu][blk_flags/2];

        bit_buffer     <<= map->cs;
        available_bits -=  map->cs;

        uint32_t symbol_size = map->sz;
        if ( i + symbol_size > page_sz ) {
            log( LOG_ERROR, spi->self.name,
                 "huffmann result overflowed buffer" );
            return 0;
        }
        memcpy( page_base + i, map->arr, symbol_size );
        i += symbol_size;
    }

    return 1;
}
int spi_huffmann_read( fastspi_inst *spi, int addr, void *buffer, int count ) {
    uint32_t csxefctrl = *(uint32_t *)(spi->func.func.config.bytes + 0x80);
    uint32_t hd_limit  = *(uint32_t *)(spi->func.func.config.bytes + 0x84) & ~0xFFFu;
    uint32_t page_sz = 1024, page, start_page, end_page;
    uint32_t end_addr, start_off, end_off, p_off, p_sz, pos;
    int st;

    /* Ensure hardware huffman decode is enabled */
    if ( ~csxefctrl & SPI_CSXE_FCTRL_HDEN ) {
        log( LOG_ERROR, spi->self.name, "huffmann read while HDEN unset" );
        goto hd_fail;
    }

    /* Determine page size */
    if ( csxefctrl & SPI_CSXE_FCTRL_PAGESZ )
        page_sz = 4096;

    /* Perform bounds check on the address */
    if ( addr + count >= hd_limit ) {
        log( LOG_ERROR, spi->self.name, "huffmann read beyond bounds: %08X to %08X",
                addr, addr+count );
        goto hd_fail;
    }
    /* Acquire the lookup table */
    spi_cache_hdlut( spi );

    end_addr = addr + count;

    start_page = addr / page_sz;
    start_off  = addr % page_sz;
    end_page   = (end_addr + page_sz - 1) / page_sz;
    end_off    = end_addr % page_sz;
    if ( end_off == 0 )
        end_off = page_sz;

    /* Perform bounds check against cached LUT size */
    if ( end_page > spi->cached_hdlut_size ) {
        log( LOG_ERROR, spi->self.name, "huffmann read beyond LUT bounds: %08X to %08X",
             addr, addr+count );
        goto hd_fail;
    }

    p_off = start_off;

    for ( page = start_page, pos = 0;
          page < end_page;
          page++, p_off = 0, pos+=p_sz ) {

        p_sz = count - pos;

        if ( p_sz > page_sz - p_off )
            p_sz = page_sz - p_off;

           if ( p_off || p_sz != page_sz ) {
               st = spi_huffman_read_page( spi, page, spi->page_buf );
               memcpy( buffer + pos, spi->page_buf + p_off, p_sz );
           } else
               st = spi_huffman_read_page( spi, page, buffer + pos );
           if (!st)
               goto hd_fail;
    }


    return 1;
  hd_fail:
    memset( buffer, 0xFF, count );
    return 1;
}

int spi_direct_read( fastspi_inst *spi, int addr, void *buffer, int count ) {
    spi_read_csme( spi, addr, count, buffer );
    return 0;


}

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
        case 0x0:
            *buf = 0;
            *buf |= (spi->spi_regions[2].base >> 12u) & 0x7FFFu;
            *buf |= (spi->spi_regions[2].limit << 4u) & 0x7FFF0000u;
            break;
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
        case 0xD8:
            *buf = spi->spi_hdcompoff;
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
        case 0xD8:
            spi->spi_hdcompoff = *buf;
            break;
        default:
            log(LOG_WARN, spi->self.name, "write 0x%03x count:%i %x", addr, count, *buf);
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

int spi_read_csme( fastspi_inst *spi, int start, int count, void *buffer ) {
    //TODO: Support BRS
    uint32_t base  = spi->spi_regions[2].base;
    uint32_t limit = spi->spi_regions[2].limit;
    uint32_t rda = start + base;
    if ( rda >= limit || (rda + count) >= limit ) {
        log(LOG_ERROR, spi->self.name,
                "Out of bounds read to CSME region with offset:%i count:%i",
                 start, count);
        return 0;
    }
    return spi_readimg( spi, rda, count, buffer );

}

int spi_readimg( fastspi_inst *spi, int start, int count, void *buffer ) {
    off_t ls; ssize_t rc;
    ls = lseek(spi->spi_image_file, start, SEEK_SET);
    if (ls == -1)
        goto flash_err;
    rc = read(spi->spi_image_file, buffer, count);
    if (rc != count)
        goto flash_err;
    return 1;
flash_err:
    log(LOG_ERROR, spi->self.name, "SPI error: %s", strerror(errno));
    return 0;

}