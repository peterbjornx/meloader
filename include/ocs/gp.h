//
// Created by pbx on 26/04/20.
//

#ifndef MELOADER_GP_H
#define MELOADER_GP_H

#include <stdint.h>
#include <stddef.h>
#include "ocs/gpdma.h"
#include "devreg.h"

typedef struct {
    const char *name;
    gpdma_state gp_gpdma;
} ocs_gp;

int gp_read( ocs_gp *a, int addr, void *buffer, int count );
int gp_write( ocs_gp *a, int addr, const void *buffer, int count );
void gp_init( device_instance *parent, ocs_gp *a );

#endif //MELOADER_GP_H
