//
// Created by pbx on 15/04/19.
//
#include "mipitrace.h"
#include "printf.h"

const char *mipi_tmsg_types[] = {
        "Dn", "DnM", "DnTS", "DnMTS", "USER", "USER_TS", "FLAG", "FLAG_TS",
        "MERR"
};

#define TRACEBUF_SIZE 64

int tbuf_master = -1;
int tbuf_channel = -1;
int tbuf_pos = 0;

struct th_msg tbuf_msgs[TRACEBUF_SIZE];

void sven_decode(int master, int channel, int header, int size, char *data) {
    int type = header & 0xF;
    int severity = (header >> 4) & 0x7;
    int unit = (header >> 12) & 0xF;
    int module = (header >> 16) & 0xFF;
    int subtype = (header >> 24) & 0xFF;
    switch( type ) {
        case 2:
            data[size] = 0;
            mel_printf("[sven] debug_string sev:%02x unit:%02x module:%02x sub:%02x \"%s\"\n", severity, unit, module,subtype, data);
            break;
        case 3:
            mel_printf("[sven] catalog_msg  sev:%02x unit:%02x module:%02x sub:%02x %08x%08x\n", severity, unit, module,subtype, *(uint32_t *)(data+8), *(uint32_t *)(data));
            break;

    }
}

char tbuf_data[TRACEBUF_SIZE * 4];
void trace_decode() {
    int ipos;
    int dpos=0;
    int type;
    int size;
    if (tbuf_msgs[tbuf_pos - 1].type != MT_FLAG)
        return;
    if (tbuf_msgs[0].type != MT_DnTS) {
        mel_printf("[mipi] Trace packet did not start with DnTS\n");
        tbuf_pos = 0;
        return;
    } else if (tbuf_msgs[0].size != 4) {
        mel_printf("[mipi] Trace packet did not start with 32 bit pkt\n");
        tbuf_pos = 0;
        return;
    } else if (tbuf_msgs[1].size != 2) {
        mel_printf("[mipi] Trace packet size is not 16 bit\n");
        tbuf_pos = 0;
        return;
    } else if (tbuf_msgs[tbuf_pos - 1].d32) {
        mel_printf("[mipi] Trace flags is not zero\n");
        tbuf_pos = 0;
        return;
    }
    type = tbuf_msgs[0].d32;
    size = tbuf_msgs[1].d16;
    mel_printf("[mipi] Packet from %i:%i with type 0x%08X and size %i\n",
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
        mel_printf("[mipi] Trace buffer overflow, dropping packet!\n");
        return;
    }
    if (tbuf_master != master || tbuf_channel != chan) {
        if (tbuf_master != -1)
            mel_printf(
                    "[mipi] Dropping trace buffer because of interleaved masters\n");
        tbuf_pos = 0;
        tbuf_master = master;
        tbuf_channel = chan;
    }
    tbuf_msgs[tbuf_pos++] = msg;
    trace_decode();
}