//
// Created by pbx on 20/06/19.
//

#ifndef MELOADER_DEVREG_H
#define MELOADER_DEVREG_H

#include "cfg_file.h"

typedef struct device_instance_s device_instance;
typedef struct device_type_s device_type;
typedef device_instance *(*device_spawn)( const cfg_file *file, const cfg_section *section );

struct device_type_s {
    device_type      *next;
    const char       *name;
    device_spawn      spawn;
};

struct device_instance_s {
    device_instance  *next;
    const char       *name;
    void             *impl;
    void             (*process)( device_instance *inst );
};

void device_type_register( device_type *type );

const device_type *device_type_find( const char *name );

void initialize_devices( const cfg_file *file );

void device_register( device_instance *device );

device_instance *device_find( const char *name );

void device_process();

#endif //MELOADER_DEVREG_H
