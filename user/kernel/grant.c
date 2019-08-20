//
// Created by pbx on 19/04/19.
//

#include <stdint.h>
#include <string.h>
#include "log.h"
#include "printf.h"
#include "grant.h"

typedef struct {
    mg_desc_t *desc;
    size_t     size;
} mg_synclist_par;

mg_desc_t *mg_list;
int        mg_count;
void dump_mglist()
{
    int i;
    for( i = 0; i < mg_count; i++ ) {
        if ( ~mg_list[i].flags & MG_EXISTS )
            continue;
        mel_printf("[grnt] g:%i hnd:%i buf: 0x%08x size:0x%08x seg:%04x ",
                i,
                mg_list[i].obj,
                mg_list[i].buffer,
                mg_list[i].size,
                mg_list[i].segment);
        if ( mg_list[i].flags & MG_FIELD1_USED )
            mel_printf("F1 ");
        else
            mel_printf("   ");
        if ( mg_list[i].flags & MG_BY_PID )
            mel_printf("PID ");
        else
            mel_printf("    ");
        if ( mg_list[i].flags & MG_FLAG1 )
            mel_printf("FL1 ");
        else
            mel_printf("    ");
        if ( mg_list[i].flags & MG_FLAG2 )
            mel_printf("FL2\n");
        else
            mel_printf("   \n");

    }
}
int sys_mg_synclist( mg_synclist_par *par )
{
    log(LOG_DEBUG, "krnl", "sys_mg_synclist( 0x%08x, %i )", par->desc, par->size);
    mg_list = par->desc;
    mg_count = par->size;
    return 0;
}