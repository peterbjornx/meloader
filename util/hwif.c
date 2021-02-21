//
// Created by pbx on 16/11/19.
//


#include <netdb.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include "log.h"
#include "cfg_file.h"
#include "hwif.h"

int hwif_fd;

void hwif_load_config( cfg_section *section ) {

  const char *host;

  if ( !section )
    return;



}

void hwif_connect( const char *host, int port ) {
    int r;
    struct hostent *_host;
    struct sockaddr_in addr;

    _host = gethostbyname( host );
    logassert( _host != NULL, "hwif", "Could not resolve host %s", host );

    hwif_fd = socket( AF_INET, SOCK_STREAM, 0 );
    logassert( hwif_fd >= 0, "hwif", "Could not allocate socket fd: %s", strerror( errno ) );

    memset( &addr, 0 , sizeof addr );
    addr.sin_family = AF_INET;
    addr.sin_port   = htons( port );
    addr.sin_addr   = *(struct in_addr *)_host->h_addr;

    r = connect(hwif_fd, (const struct sockaddr *) &addr, sizeof addr );
    logassert( r >= 0, "hwif", "Could not connect to %s:%i : %s", host, port, strerror( errno ) );

}

void hwif_write_fully( const void *buffer, int count ) {

    ssize_t rd = 0, rr;

    while ( rd < count ) {
        rr = write( hwif_fd, buffer + rd, count - rd );
        logassert( rr >= 0 || errno == EAGAIN, "hwif", "Could not write: %s", strerror( errno ) );
        rd += rr;
    }

}

void hwif_read_fully( void *buffer, int count ) {

    ssize_t rd = 0, rr;

    while ( rd < count ) {
        rr = read( hwif_fd, buffer + rd, count - rd );
        logassert( rr >= 0 || errno == EAGAIN, "hwif", "Could not read: %s", strerror( errno ) );
        rd += rr;
    }

}

int hwif_handle_read( void *buffer, int count ) {

    int status;

    hwif_read_fully( &status, sizeof status );

    if ( status != 0 )
        return status;

    hwif_read_fully( buffer, count );

    return 0;

}

int hwif_handle_write( ) {

    int status;

    hwif_read_fully( &status, sizeof status );

    return status;

}

int hwif_sb_write(
    hwif_sbaddr addr,
    uint32_t  offset,
    const void *buffer,
    int size ) {
    int pkt = HWIF_PACKET_SB_WRITE;
    hwif_write_fully( &pkt,    sizeof pkt );
    hwif_write_fully( &addr,   sizeof addr   );
    hwif_write_fully( &offset, sizeof offset );
    hwif_write_fully( &size  , sizeof size );

    hwif_write_fully( buffer , size );
    log(LOG_INFO, "hwif",
        "sideband_write( rs=0x%02x, endp=0x%02X, func=0x%02X, bar=0x%01X, opcode=%02X, offset=0x%04X, size=%i, data=%08X )",
        addr.rs, addr.endpt, addr.func, addr.bar, addr.wr_op, offset, size, *(uint32_t*) buffer);
    return hwif_handle_write();
}

int hwif_sb_read(
    hwif_sbaddr addr,
    uint32_t  offset,
    void *buffer,
    int size ) {
    int r;
    int pkt = HWIF_PACKET_SB_READ;
    hwif_write_fully( &pkt,    sizeof pkt );
    hwif_write_fully( &addr,   sizeof addr   );
    hwif_write_fully( &offset, sizeof offset );
    hwif_write_fully( &size  , sizeof size );
    r = hwif_handle_read( buffer , size );
    log(LOG_INFO, "hwif",
        "sideband_read( rs=0x%02x, endp=0x%02X, func=0x%02X, bar=0x%01X, opcode=%02X, offset=0x%04X, size=%i ) # result: 0x%08X",
        addr.rs, addr.endpt, addr.func, addr.bar, addr.wr_op, offset, size, *(uint32_t*) buffer);
    return r;
}

int hwif_mm_write(
    uint32_t    addr,
    const void *buffer,
    int size ) {
    log(LOG_INFO, "hwif", "hwif.memory_write( 0x%08X, %i, 0x%08X )", addr, size, *(uint32_t*) buffer);
    int pkt = HWIF_PACKET_MM_WRITE;
    hwif_write_fully( &pkt,    sizeof pkt );
    hwif_write_fully( &addr,   sizeof addr   );
    hwif_write_fully( &size  , sizeof size );
    hwif_write_fully( buffer , size );
    return hwif_handle_write();
}

int hwif_mm_read(
    uint32_t    addr,
    void *buffer,
    int size ) {
    int pkt = HWIF_PACKET_MM_READ, r;
    hwif_write_fully( &pkt,    sizeof pkt );
    hwif_write_fully( &addr,   sizeof addr   );
    hwif_write_fully( &size  , sizeof size );
    r = hwif_handle_read( buffer , size );
    log(LOG_INFO, "hwif", "hwif.memory_read( 0x%08X, %i ) # result: 0x%08X", addr, size, *(uint32_t*) buffer);
    return r;
}

void hwif_disconnect() {
    close( hwif_fd );
}