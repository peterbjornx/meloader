//
// Created by pbx on 21/04/19.
//

#ifndef MELOADER_LOG_H
#define MELOADER_LOG_H
#include <stdarg.h>
#define LOG_TRACE (0)
#define LOG_DEBUG (1)
#define LOG_INFO  (2)
#define LOG_WARN  (3)
#define LOG_ERROR (4)
#define LOG_FATAL (5)
#define LOG_METRC (6)

void vlog( int level, const char *module, const char *format, va_list list );
void log( int level, const char *module, const char *format, ...);
void fatal( const char *module, const char *format, ...);
void logassert( int cond, const char *module, const char *format, ... );

#endif //MELOADER_LOG_H
