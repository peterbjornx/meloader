//
// Created by pbx on 15/04/19.
//
#include "log.h"
#include <string.h>
#include <stdlib.h>
#include "cfg_file.h"
#include "mipitrace.h"
#include "printf.h"

const char *mipi_tmsg_types[] = {
        "Dn", "DnM", "DnTS", "DnMTS", "USER", "USER_TS", "FLAG", "FLAG_TS",
        "MERR"
};

#define TRACEBUF_SIZE 4096

int tbuf_master = -1;
int tbuf_channel = -1;
int tbuf_pos = 0;

struct th_msg tbuf_msgs[TRACEBUF_SIZE];

static void print_multiline( const char *module, const char *str ) {
    char *buf = strdup(str);
    char *t;
    if (!module)
        module = "sven";
    while ( str ) {
        t = strchr(str, '\n');
        if ( t )
            *t = 0;
        if ( *str )
            log( LOG_METRC, module, "%s", str );
        str = t;
    }
    t = strchr(buf,'\n');
    free(buf);
}

static cfg_file *sven_file = NULL;
static char buffer[40];
static char sven_buffer[640];

void sven_load( const char *path ) {
    sven_file = load_config( path );
}
extern int trig;
void sven_getfmt( char *data ) {
    const cfg_section *msg = NULL;
    const char *fmt = NULL;
    uint32_t *idata = (uint32_t *) data;
    uint32_t id_low = *( uint32_t * ) ( data );
    uint32_t id_high = *( uint32_t * ) ( data + 4);
    if ( id_low == 0x30400 ) {
        trig = 1;
        log(LOG_WARN,"toctou","Trigger seen!");
    }
    mel_snprintf( buffer, 40, "msg_0x%08x%08x", id_high, id_low );
    if ( sven_file )
        msg = cfg_find_section( sven_file, buffer );
    if ( msg )
        fmt = cfg_find_string( msg, "message" );
    if ( !fmt )
        mel_snprintf( sven_buffer, 640, "%08X%08X%08X", idata[0], idata[1], idata[2] );
    else
        mel_snprintf( sven_buffer, 640, fmt, idata[2], idata[3], idata[4], idata[5], idata[6] );
    print_multiline( NULL, sven_buffer );
}

void sven_decode(int master, int channel, int header, int size, char *data) {
    int type = header & 0xF;
    int severity = (header >> 4) & 0x7;
    int unit = (header >> 12) & 0xF;
    int module = (header >> 16) & 0xFF;
    int subtype = (header >> 24) & 0xFF;
    switch( type ) {
        case 2:
            data[size] = 0;
            log(LOG_TRACE, "sven", "debug_string sev:%02x unit:%02x module:%02x sub:%02x", severity, unit, module,subtype);
            print_multiline( NULL, data );
            break;
        case 3:
            log(LOG_TRACE, "sven", "catalog_msg  sev:%02x unit:%02x module:%02x sub:%02x %08x%08x", severity, unit, module,subtype, *(uint32_t *)(data+4), *(uint32_t *)(data));
            sven_getfmt( data );
            break;

    }
}

char tbuf_data[TRACEBUF_SIZE * 8];
void trace_decode() {
    int ipos;
    int dpos=0;
    int type;
    int size;
    if (tbuf_msgs[tbuf_pos - 1].type != MT_FLAG)
        return;
    if (tbuf_msgs[0].type != MT_DnTS) {
        log(LOG_ERROR, "mipi", "Trace packet did not start with DnTS");
        tbuf_pos = 0;
        return;
    } else if (tbuf_msgs[0].size != 4) {
        log(LOG_ERROR, "mipi", "Trace packet did not start with 32 bit pkt");
        tbuf_pos = 0;
        return;
    } else if (tbuf_msgs[1].size != 2) {
        log(LOG_ERROR, "mipi", "Trace packet size is not 16 bit");
        tbuf_pos = 0;
        return;
    } else if (tbuf_msgs[tbuf_pos - 1].d32) {
        log(LOG_ERROR, "mipi", "Trace flags is not zero");
        tbuf_pos = 0;
        return;
    }
    type = tbuf_msgs[0].d32;
    size = tbuf_msgs[1].d16;
    log(LOG_TRACE, "mipi", "Packet from %i:%i with type 0x%08X and size %i",
               tbuf_master,tbuf_channel,type,size);
    ipos = 2;
    while ( (dpos+4) <= size ) {
        *((uint32_t *)(tbuf_data + dpos)) = tbuf_msgs[ipos].d32;
        dpos += 4;
        ipos ++;
    }
    while ( (dpos+2) <= size ) {
        *((uint16_t *)(tbuf_data + dpos)) = tbuf_msgs[ipos].d16;
        dpos += 2;
        ipos ++;
    }
    while ( dpos < size ) {
        *((uint8_t *)(tbuf_data + dpos)) = tbuf_msgs[ipos].d8;
        dpos ++;
        ipos ++;
    }
    tbuf_pos = 0;
    tbuf_master = -1;
    sven_decode(tbuf_master, tbuf_channel, type, size, tbuf_data );
}

void trace_msg(int master, int chan, struct th_msg msg) {

    if (tbuf_pos > TRACEBUF_SIZE) {
        log(LOG_ERROR, "mipi", "Trace buffer overflow, dropping packet!");
        return;
    }
    if (tbuf_master != master || tbuf_channel != chan) {
        if (tbuf_master != -1)
            log(LOG_ERROR, "mipi", "Dropping trace buffer because of interleaved masters");
        tbuf_pos = 0;
        tbuf_master = master;
        tbuf_channel = chan;
    }
    tbuf_msgs[tbuf_pos++] = msg;
    trace_decode();
}