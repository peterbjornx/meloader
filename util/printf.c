/**
 * @file crt/printf.c
 *
 * Part of posnk kernel
 *
 * Written by Peter Bosch <peterbosc@gmail.com>
 *
 */

#include <unistd.h>
#include <fcntl.h>
#include <stdint.h>
#include <string.h>
#include <stdarg.h>
#include <assert.h>
#include <numfmt.h>

#define PF_PRINT	(0)
#define PF_FSPEC	(1)
#define PF_WIDTH	(2)
#define PF_PRECI	(3)
#define PF_AWIDT	(4)
#define PF_LMOD		(5)
#define PF_PRESC	(6)
#define PF_TYPE		(7)

#define PF_DEFT		(0)
#define PF_SHORT	(1)
#define PF_CHAR		(2)
#define	PF_LONG		(3)
#define PF_LONGLONG	(4)
#define PF_SIZE		(5)
#define PF_PTRDIFF	(6)
#define PF_INTMAX	(7)

/**
 * @brief Formatted print function
 * @param putch		The function used to store/output the formatted output
 * @param arg		A pointer that will be passed to the putch parameter
 * @param format	The format string
 * @param list		The varargs list
 */
int mel_vpprintf(	void (*putch)( void *arg, char c, int count ),
                 void *arg,
                 const char *format,
                 va_list list )
{
    char fch;
    char nfbuf[65];
    char *sval;
    char c;
    int state = PF_PRINT;
    int nflags;
    int width;
    int size;
    int base;
    int presc;
    int count = 0;
    intmax_t sival;
    uintmax_t uival;

    assert( format != NULL );

    while ( ( fch = *format++ ) ) {
        switch ( state ) {
            case PF_PRINT:
                if ( fch == '%' ) {
                    state = PF_FSPEC;
                    width = 0;
                    nflags = 0;
                    size = PF_DEFT;
                } else
                    putch( arg, fch, count++ );
                break;
            case PF_FSPEC:
                if ( fch == '-' )
                    break;// LEFT JUSTIFIED
                else if ( fch == '+' ) {
                    nflags |= NF_SGNPLUS;
                    break;
                } else if ( fch == ' ' )
                    break;// SPACE PREPEND IF NO SGN
                else if ( fch == '#' )
                    break;// ALTERNATE FMT CHAR
                else if ( fch == '0' ) {
                    nflags |= NF_ZEROPAD;
                    break;
                }
            case PF_WIDTH:
                state = PF_WIDTH;
                if ( fch >= '0' && fch <= '9' ) {
                    width = width * 10 + fch - '0';
                    break;
                } else if ( fch == '*') {
                    width = va_arg( list, int );
                    state = PF_AWIDT;
                    break;
                }
            case PF_AWIDT:
                state = PF_AWIDT;
                if ( fch == '.' ) {
                    state = PF_PRESC;
                    break;
                }
            case PF_LMOD:
                state = PF_LMOD;
                if (  fch == 'h' && size != PF_SHORT) {
                    size = PF_SHORT;
                    break;
                } else if ( fch == 'h' ) {
                    size = PF_CHAR;
                    state = PF_TYPE;
                    break;
                } else if ( fch == 'l' && size != PF_LONG ) {
                    size = PF_LONG;
                    break;
                } else if ( fch == 'l' ) {
                    size = PF_LONGLONG;
                    state = PF_TYPE;
                    break;
                } else if ( fch == 'j' ) {
                    size = PF_INTMAX;
                    state = PF_TYPE;
                    break;
                } else if ( fch == 'z' ) {
                    size = PF_SIZE;
                    state = PF_TYPE;
                    break;
                } else if ( fch == 't' ) {
                    size = PF_PTRDIFF;
                    state = PF_TYPE;
                    break;
                }
                assert( fch != 'L' );
            case PF_TYPE:
                state = PF_TYPE;
                if ( fch == 'i' || fch == 'd' ) {
                    switch ( size ) {
                        case PF_CHAR:
                            sival = ( signed char )
                                    va_arg(	list,
                                               int);
                            break;
                        case PF_SHORT:
                            sival = ( signed short )
                                    va_arg(	list,
                                               int);
                            break;
                        case PF_DEFT:
                            sival = va_arg(	list,
                                               int );
                            break;
                        case PF_LONG:
                            sival = va_arg( list,
                                            signed long );
                            break;
                        case PF_LONGLONG:
                            sival = va_arg(	list,
                                               signed long long );
                            break;
                        case PF_INTMAX:
                            sival = va_arg( list,
                                            intmax_t );
                            break;
                        case PF_SIZE:
                            sival = va_arg( list,
                                            size_t );
                            break;
                        case PF_PTRDIFF:
                            sival = va_arg( list,
                                            ptrdiff_t );
                            break;
                        default:
                            assert( !"bad size specifier "
                                     "in printf" );
                    }
                    numfmt_signed(
                            sival,
                            nflags,
                            width,
                            10,
                            nfbuf,
                            sizeof nfbuf);
                    sval = nfbuf;
                } else if (	fch == 'o' ||
                               fch == 'x' ||
                               fch == 'u' ||
                               fch == 'X' ||
                               fch == 'p' ) {
                    if (	fch == 'p' ) {
                        uival = (uintmax_t) (uintptr_t)
                                va_arg( list, void * );
                    } else switch ( size ) {
                            case PF_CHAR:
                                uival = ( unsigned char )
                                        va_arg(list, int);
                                break;
                            case PF_SHORT:
                                uival = ( unsigned short )
                                        va_arg(list, int);
                                break;
                            case PF_DEFT:
                                uival = va_arg( list,
                                                unsigned int );
                                break;
                            case PF_LONG:
                                uival = va_arg( list,
                                                unsigned long );
                                break;
                            case PF_LONGLONG:
                                sival = va_arg(	list,
                                                   unsigned long long );
                                break;
                            case PF_INTMAX:
                                uival = va_arg( list,
                                                uintmax_t );
                                break;
                            case PF_SIZE:
                                uival = va_arg( list,
                                                size_t );
                                break;
                            case PF_PTRDIFF:
                                uival = (uintmax_t)
                                        va_arg( list,
                                                ptrdiff_t );
                                break;
                            default:
                                assert( !"bad size specifier "
                                         "in printf" );
                        }
                    switch ( fch ) {
                        case 'x'://TODO:lcase hex
                        case 'X':
                        case 'p':
                            base = 16;
                            break;
                        case 'u':
                            base = 10;
                            break;
                        case 'o':
                            base = 8;
                            break;
                    }
                    numfmt_unsigned( uival,
                                     nflags,
                                     width,
                                     base,
                                     nfbuf,
                                     sizeof nfbuf );
                    sval = nfbuf;
                } else if ( fch == 'c' ) {
                    assert( size == PF_DEFT );
                    putch(	arg,
                              ( char ) va_arg( list, int ),
                              count++ );
                } else if ( fch == 's' ) {
                    assert( size == PF_DEFT );
                    sval = va_arg( list, char * );
                } else if ( fch == 'n' ) {
                    assert( !"UNSAFE %%n specifier used" );
                } else if ( fch == '%' ) {
                    putch( arg, '%', count++ );
                } else {
                    assert( !"UNKNOWN concersion" );
                }
                if (	fch == 'd' ||
                        fch == 'i' ||
                        fch == 'x' ||
                        fch == 'X' ||
                        fch == 'p' ||
                        fch == 'o' ||
                        fch == 'u' ||
                        fch == 's' ) {
                    while ( ( c = *sval++ ) != 0 )
                        putch( arg, c, count++ );
                }
                state = PF_PRINT;
                break;
            case PF_PRESC:
                if ( fch >= '0' && fch <= '9' ) {
                    presc = presc * 10 + fch - '0';
                    break;
                } else if ( fch == '*') {
                    presc = va_arg( list, int );
                    state = PF_LMOD;
                    break;
                }
            default:
                assert(!"INVALID PF STATE");
                break;
        }
    }
    return count;
}

typedef struct {
    char	*str;
    size_t	 len;
} __snprintf_str;

void __snprintf_putch( void * arg, char c, int count )
{
    __snprintf_str *str = ( __snprintf_str * ) arg;
    assert( str != NULL );

    if ( count >= str->len - 1 )
        return;

    str->str[ count ] = c;

}

/**
 * @brief Varargs version of snprintf
 * @param str		Output string
 * @param size		Size limit for the string
 * @param format	Format string
 * @param list		Varargs list
 * @see vpprintf
 */
int mel_vsnprintf(	char *str, size_t size, const char *format, va_list list )
{
    __snprintf_str arg;
    int result;
    assert( str != NULL );

    arg.str = str;
    arg.len = size;

    result = mel_vpprintf( __snprintf_putch, &arg, format, list );

    if ( result > size - 1 )
        result = size - 1;

    str[ result ] = 0;

    return result;

}

/**
 * @brief Varargs version of sprintf
 * @param str		Output string
 * @param format	Format string
 * @param list		Varargs list
 * @deprecated *Very* unsafe function
 * @see vpprintf
 */
int mel_vsprintf( char *str, const char *format, va_list list )
{
    assert(!"Attempting to use unsafe interface!");
    return 0;
}

/**
 * @brief printf to string
 * @param str		Output string
 * @param format	Format string
 * @param ...		Varargs for values
 * @deprecated *Very* unsafe function
 */
int mel_sprintf( char *str, const char *format, ... )
{
    int result;
    va_list list;

    va_start( list, format );

    result = mel_vsprintf( str, format, list );

    va_end( list );

    return result;

}

/**
 * @brief printf to size limited string
 * @param str		Output string
 * @param size		Size limit for the string
 * @param format	Format string
 * @param ...		Varargs for values
 * @see vpprintf
 */
int mel_snprintf( char *str, size_t size, const char *format, ... )
{
    int result;
    va_list list;

    va_start( list, format );

    result = mel_vsnprintf( str, size, format, list );

    va_end( list );

    return result;
}

void __printf_putch( void * arg, char c, int count )
{
    write(STDERR_FILENO, &c, 1);
}

/**
 * Varargs version of printf
 * @param format	Format string
 * @param list		Varargs list
 * @see vpprintf
 * @see printf
 */
int mel_vprintf( const char *format, va_list list )
{

    return mel_vpprintf( __printf_putch, NULL, format, list );

}

/**
 * Formatted print to kernel console
 * @param format	Format string
 * @param ...		Varargs for values
 * @see vpprintf
 */
int mel_printf( const char *format, ... )
{

    int result;
    va_list list;

    va_start( list, format );

    result = mel_vprintf( format, list );

    va_end( list );

    return result;

}
