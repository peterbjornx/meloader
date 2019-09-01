//
// Created by pbx on 01/09/19.
//

#ifndef MELOADER_DFXAGG_H
#define MELOADER_DFXAGG_H

#include "devreg.h"
#include "sideband.h"

#define CONSENT_DEBUG_NOTIFICATION (1u<<31u)
#define CONSENT_LOCK_PRIVACY_OPT   (1u<<30u)
#define CONSENT_PRIVACY_OPT        (1u<<0u)

typedef struct {
    device_instance self;
    sideband_dev sb;
    int sai;
    uint32_t consent;
    uint32_t personality;
    uint64_t status;
    uint64_t puid;
} dfxagg_inst;

#endif //MELOADER_DFXAGG_H
