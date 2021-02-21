//
// Created by pbx on 02/09/19.
//

#ifndef MELOADER_UNICORNCPU_H
#define MELOADER_UNICORNCPU_H

#include <libia32.h>
#include "fsb.h"

typedef struct {
    mia_fsb fsb;
    device_instance self;
    libia32_corecplx *engine;

} libia32cpu_inst;

#endif //MELOADER_UNICORNCPU_H
