//
// Created by pbx on 21/04/19.
//

#include <stdarg.h>
#include <stdlib.h>
#include "printf.h"
#include "log.h"

char log_buffer[1024];

int log_level = LOG_TRACE;

const char *level_names[]
    = {"TRACE", "DEBUG", "INFO ", "WARN ", "ERROR", "FATAL", "METRC"};

void vlog( int level, const char *module, const char *format, va_list list ) {

    if ( level >= log_level ) {

        mel_vsnprintf( log_buffer, 1024, format, list );

        mel_printf( "[%s] %s: %s\n",
                    level_names[level],
                    module,
                    log_buffer );

    }

}


void log( int level, const char *module, const char *format, ...) {

    va_list  list;
    va_start(list, format);

    vlog( level, module, format, list );

    va_end(list);
}

void fatal( const char *module, const char *format, ...) {

    va_list  list;
    va_start(list, format);

    vlog( LOG_FATAL, module, format, list );

    va_end(list);

    exit( EXIT_FAILURE );

}

void logassert( int cond, const char *module, const char *format, ... ) {

    va_list  list;
    va_start(list, format);

    if ( cond )
        return;

    vlog( LOG_FATAL, module, format, list );

    va_end(list);

    exit( EXIT_FAILURE );

}