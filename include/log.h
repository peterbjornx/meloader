//
// Created by pbx on 21/04/19.
//

#ifndef MELOADER_LOG_H
#define MELOADER_LOG_H

#define LOG_TRACE (0)
#define LOG_DEBUG (1)
#define LOG_INFO  (2)
#define LOG_WARN  (3)
#define LOG_ERROR (4)
#define LOG_FATAL (5)

void log( int level, const char *module, const char *format, ...);

#endif //MELOADER_LOG_H
