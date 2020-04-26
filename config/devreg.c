//
// Created by pbx on 20/06/19.
//
#include <string.h>
#include <stdlib.h>
#include "log.h"
#include "devreg.h"

static device_type *type_list;
static device_instance *device_list;

const device_type *device_type_find( const char *name ) {
    device_type *cur;
    for ( cur = type_list; cur; cur = cur->next) {
        if ( strcmp( name, cur->name ) == 0 )
            break;
    }
    return cur;
}

void device_type_register( device_type *type ) {
    if ( device_type_find( type->name ) ) {
        log(LOG_FATAL, "devreg", "Tried to redefine device type %s", type);
        exit(EXIT_FAILURE);
    }
    type->next = type_list;
    type_list = type;
}

device_instance *device_find( const char *name ) {
    device_instance *cur;
    for ( cur = device_list; cur; cur = cur->next) {
        if ( strcmp( name, cur->name ) == 0 )
            break;
    }
    return cur;
}

void device_register( device_instance *device ) {
    if ( device_find( device->name ) ) {
        log(LOG_FATAL, "devreg", "Tried to redefine device %s", device);
        exit(EXIT_FAILURE);
    }
    device->next = device_list;
    device_list = device;
}

void initialize_devices( const cfg_file *file ) {
    const cfg_section *section;
    const device_type *type;
    const char *typename;
    for ( section = file->first_section; section; section = section->next ) {
        if ( strcmp( section->type, "device" ) != 0 )
            continue;
        typename = cfg_find_string( section, "type" );
        if (!typename) {
            log(LOG_FATAL, "devreg", "Error instantiating device %s: no type field", section->name);
            exit(EXIT_FAILURE);
        }
        type = device_type_find( typename );
        if (!type) {
            log(LOG_FATAL, "devreg", "Error instantiating device %s: unknown type %s",
                    section->name, typename);
            exit(EXIT_FAILURE);
        }
        type->spawn( file, section );
    }
}

void device_process() {
    device_instance *cur;
    for ( cur = device_list; cur; cur = cur->next) {
        if ( cur->process )
            cur->process( cur );
    }
}