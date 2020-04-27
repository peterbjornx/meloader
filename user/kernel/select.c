//
// Created by pbx on 31/08/19.
//
#include "log.h"
#include <stdint.h>
#include <unistd.h>

/*
 * selrecv_str_t   struc ; (sizeof=0x6C, mappedto_31)
00000000                                         ; XREF: srv_task/r
00000000                                         ; srv_task/r ...
00000000 field_0         dq ?
00000008 field_8         dq ?                    ; XREF: srv_task+15A/o
00000008                                         ; srv_task+415/o ...
00000010 field_10        dq ?                    ; XREF: srv_task+16C/o
00000010                                         ; srv_task+192/w ...
00000018 in_msg          selrecv_msg_t ?         ; XREF: srv_task+B9/w
00000018                                         ; srv_task+C3/w ...
00000058 flags           dd ?                    ; XREF: srv_task+3F/w
00000058                                         ; srv_task+6B/w ...
0000005C timeout         dq ?                    ; XREF: srv_task+110/w
0000005C                                         ; srv_task+116/w ...
00000064 result_type     dd ?                    ; XREF: srv_task+1EB/r
00000064                                         ; srv_task+1F4/r ...
00000068 ret_val         dd ?                    ; XREF: srv_task:loc_F6FC/r
00000068                                         ; srv_task:loc_F70B/r ...
0000006C selrecv_str_t   ends
 */
typedef struct __attribute__((packed)){
    uint64_t bitfield0;
    uint64_t bitfield1;
    uint64_t bitfield2;
    union {
        uint8_t raw_msg[0x40];
    };
    uint32_t flags;
    uint64_t timeout;
    uint32_t result_type;
    uint32_t ret_val;
} select_receive_par;

int sys_select_receive( select_receive_par *par )
{
    if ( par->flags == 0 && par->bitfield2 == 0 && par->bitfield1 == 0 && par->bitfield0 == 0 ) {
        usleep( par->timeout * 1000 );
    } else
        log(LOG_DEBUG, "krnl", "sys_select_receive( bitfield0=0x%016llx, bitfield1=0x%016llx, bitfield2=0x%016llx, flags=0x%08x, timeout=%016llx )",
            par->bitfield0, par->bitfield1, par->bitfield2, par->flags, par->timeout);

    return 0;
}
