//
// Created by pbx on 15/04/19.
//

#ifndef MELOADER_PRINTF_H
#define MELOADER_PRINTF_H
#include <string.h>
#include <stdarg.h>

int mel_vsnprintf(	char *str, size_t size, const char *format, va_list list );

int mel_vsprintf(	char *str, const char *format, va_list list );

int mel_vprintf(	const char *format, va_list list );

int mel_sprintf(	char *str, const char *format, ... );

int mel_snprintf(	char *str, size_t size, const char *format, ... );

int mel_printf(		   const char *format, ... );

#endif //ME2MU_PRINTF_H
