//
// Created by pbx on 21/04/19.
//

#include <stdarg.h>
#include "printf.h"
#include "log.h"

char log_buffer[1024];

int log_level = LOG_INFO;

const char *level_names[]
    = {"TRACE", "DEBUG", "INFO ", "WARN ", "ERROR", "FATAL"};

void log( int level, const char *module, const char *format, ...) {

    va_list  list;
    va_start(list, format);

    if ( level < log_level ) {

        mel_vsnprintf( log_buffer, 1024, format, list );

        mel_printf( "[%s] %s: %s",
                level_names[level],
                module,
                log_buffer );

    }
    va_end(list);
}
