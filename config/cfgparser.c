//
// Created by pbx on 19/06/19.
//
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include "cfg_file.h"
#include "log.h"
#include <ctype.h>
#include <stdarg.h>

/**
 * The file currently being parsed
 */
static FILE *parse_input;

static const char *parse_file;
static int parse_line = 1, parse_col, last_line_col;

static __attribute__((__noreturn__))
void parser_fatal( const char *fmt, ... ) {
    char buffer[80];
    va_list args;
    va_start( args, fmt );
    snprintf( buffer, 80, fmt, args );
    log(LOG_FATAL, "config", "%s:%i col %i: %s",
            parse_file, parse_line, parse_col, buffer);
    exit(EXIT_FAILURE);
}

/**
 * Verify that an allocation succeeded
 */
static void ensure_mem( const void *ptr ) {
    if ( ptr )
        return;
    parser_fatal("Ran out of memory");
}

/**
 * Verify that we haven't hit EOF
 */
static void ensure_noteof( int f ) {
    if ( f != EOF )
        return;
    parser_fatal("Unexpected EOF");
}

/**
 * Accepts either a character or the end of file token
 * @return ASCII character or EOF
 */
static int accept_rawchar_or_eof(){
    int in = getc( parse_input );
    if ( in == EOF )
        return EOF;
    if ( in == '\n' ) {
        parse_line++;
        last_line_col = parse_col;
        parse_col = 0;
    } else
        parse_col++;
    return in;
}

/**
 * Accept a character, fatal error if EOF is hit
 */
static char accept_rawchar(  ) {
    int in = accept_rawchar_or_eof();
    ensure_noteof( in );
    return in;
}

/**
 * Pushes a character back into the input stream
 * Used for backtracking
 */
static void return_rawchar( char c ) {
    ensure_noteof( ungetc( c, parse_input ) );
    if ( c == '\n' ) {
        parse_line--;
        parse_col = last_line_col;
    } else
        parse_col--;

}

/**
 * Expect a character, fatal error if not match
 */
static void expect_char( char c ) {
    int in = accept_rawchar();
    if ( in != c )
        parser_fatal("Expected %c, got %c", c, in);
}

/**
 * Accept a character, return it to the input stream
 * @param c The character expected to be at the next position
 * @return 0 if the character matched, -1 if not.
 */
static int accept_char( char c ) {
    int in = accept_rawchar();
    if ( in != c ) {
        return_rawchar( in );
        return -1;
    }
    return 0;
}

/**
 * Map an escape to the character it encodes
 */
static char map_esc( char c ){
    switch ( c ) {
        case '"': return '"';
        case '\\': return '\\';
        case '\'': return '\'';
        case 'n': return '\n';
        case 'r': return '\r';
        case 't': return '\t';
        case 'b': return '\b';
        default:
            parser_fatal("Unknown escape \\%c", c);
    }
}

/**
 * Accepts a string literal from the input stream
 * @return A string pointer if successful, NULL if not.
 */
static const char *accept_string() {
    char *buffer;
    char in;
    int ptr = 0, size;

    in = accept_rawchar_or_eof();

    if ( in == EOF )
        return NULL;

    if ( in != '"' ) {
        return_rawchar( in );
        return NULL;
    }

    buffer = malloc( size = 40 );
    ensure_mem( buffer );

    while ( (in = accept_rawchar() ) != '"' ) {
        if ( in == '\\' ) {
            buffer[ptr++] = map_esc(accept_rawchar());
        } else
            buffer[ptr++] = in;
        if ( ptr == size ) {
            buffer = realloc( buffer, size *= 2 );
            ensure_mem( buffer );
        }
    }
    buffer[ptr] = 0;

    return_rawchar(in);
    expect_char( '"' );

    return buffer;
}

/**
 * Accepts a name from the input stream
 * @return A string pointer if successful, NULL if not.
 */
static const char *accept_name() {
    char *buffer;
    char in;
    int ptr = 0, size;

    in = accept_rawchar_or_eof();

    if ( in == EOF )
        return NULL;

    if ( !isalpha( in ) ) {
        return_rawchar( in );
        return NULL;
    }

    buffer = malloc( size = 40 );
    ensure_mem( buffer );

    do {
        if ( in == '\\' ) {
            buffer[ptr++] = map_esc(accept_rawchar());
        } else
            buffer[ptr++] = in;
        if ( ptr == size ) {
            buffer = realloc( buffer, size *= 2 );
            ensure_mem( buffer );
        }
    } while ( isalnum(in = accept_rawchar() ) || in == '_' );

    return_rawchar(in);

    buffer[ptr] = 0;

    return buffer;
}

/**
 * Accept a run of whitespace characters or EOF, without consuming
 * the first non-whitespace character that follows
 */
static void accept_whitespace() {
    int in;
    while ( isspace( in = accept_rawchar_or_eof() ) );
    if ( in == EOF )
        return;
    return_rawchar( in );
}

/**
 * Accepts a character literal from the input stream
 * @return A character if successful, -1 if not.
 */
static int accept_char_literal() {
    int in, val;

    in = accept_rawchar_or_eof();

    if ( in == EOF )
        return -1;

    if ( in != '\'' ) {
        return_rawchar( in );
        return -1;
    }

    in = accept_rawchar();
    if ( in == '\\' ) {
        val = map_esc(accept_rawchar());
    } else
        val = in;

    expect_char( '\'' );

    return val;
}

/**
 * Maps the character representation of a digit to its numerical value.
 * @return The value of the digit or -1 if the character was not a valid digit
 */
static int map_digit( char a ) {
    if ( a >= '0' && a <= '9' )
        return a - '0';
    else if ( a >= 'a' && a <= 'f' )
        return a - 'a' + 10;
    else if ( a >= 'A' && a <= 'F' )
        return a - 'A' + 10;
    else
        return -1;
}
/**
 * Accepts an integer literal from the input stream.
 * @param out The integer that was decoded
 * @return Zero if successful, a negative value if not
 */
static int accept_int( uint64_t *out ) {
    int in = accept_rawchar_or_eof( );
    int radix, neg = 0, val;

    if ( in == EOF )
        return -1;

    if ( in != '-' && !isdigit(in) ) {
        return_rawchar( in );
        return -1;
    }

    *out = 0;

    if ( in == '-' ) {
        in = accept_rawchar();
        if ( !isdigit(in) )
            goto unexp;
        neg = 1;
    }

    if ( in == '0' ) {
        in = accept_rawchar();
        if ( in == 'x' || in == 'X' ) {
            radix = 16;
            in = accept_rawchar();
            if (map_digit(in) < 0)
                goto unexp;
        } else if ( !isdigit(in) )
            goto done;
        else
            radix = 8;
    } else
        radix = 10;


    for ( ;; ) {
        val = map_digit( in );
        if ( val < 0 || val >= radix )
            break;
        *out = *out * radix + val;
        in = accept_rawchar();
    }

  done:
    if ( neg )
        *out = -*out;
    return_rawchar( in );
    return 0;

  unexp:
    parser_fatal("Unexpected charcter %c in integer literal", in);
}

/**
 * Accepts a numerical literal, which is either a character literal or an integer
 * literal.
 * @param num The integer that was decoded
 * @return Zero if successful, a negative value if not
 */
static int accept_number( uint64_t *num ) {
    int ch;
    ch = accept_char_literal();
    if ( ch > 0 && ch < 256 ) {
        *num = ch;
        return 0;
    }
    return accept_int( num );
}

/**
 * Accepts a configuration entry, which has the structure:
 * <name> = ( <string> | <number> | <name> )
 * @return The entry that was decoded or NULL if it was not valid
 */
static cfg_entry *accept_entry(  ) {
    const char *name;
    cfg_entry *entry;
    accept_whitespace();
    name = accept_name();
    if ( !name )
        return NULL;
    entry = malloc( sizeof(cfg_entry) );
    ensure_mem( entry );
    entry->name = name;
    accept_whitespace();
    expect_char( '=' );
    accept_whitespace();
    if ( (entry->string = accept_string()) ) {
        entry->type = CONFIG_TYPE_STRING;
        return entry;
    } else if ( accept_number( &entry->int64 ) == 0 ) {
        entry->type = CONFIG_TYPE_INT64;
        return entry;
    } else if ( (entry->string = accept_name()) ) {
        entry->type = CONFIG_TYPE_STRING;
        return entry;
    }
    log( LOG_FATAL, "config", "Expected a valid rvalue" );
    exit(EXIT_FAILURE);
}

/**
 * Accepts a configuration section, which has the structure:
 * <name> <name> { <entry> ; ... }
 * @return The section that was decoded or NULL if it was not valid
 */
static cfg_section *accept_section( ) {
    cfg_section *section;
    cfg_entry *entry, *last_entry = 0;
    const char *name, *type;
    accept_whitespace();
    type = accept_name();
    if ( !type )
        return NULL;
    accept_whitespace();
    name = accept_name();
    if ( !name ) {
        log( LOG_FATAL, "config", "Expected a valid name" );
        exit(EXIT_FAILURE);
    }
    section = malloc( sizeof(cfg_section) );
    ensure_mem( section );
    accept_whitespace();
    expect_char( '{' );
    section->type = name;
    section->name = name;
    section->first_entry = 0;
    do {
        entry = accept_entry();
        if ( !entry ) {
            log( LOG_FATAL, "config", "Expected a valid cfg_entry" );
            exit(EXIT_FAILURE);
        }
        if ( !last_entry ) {
            section->first_entry = entry;
        } else {
            last_entry->next = entry;
        }
        last_entry = entry;
        accept_whitespace();
        if ( accept_char(';') != 0 ) {
            log( LOG_FATAL, "config", "Expected a ;" );
            exit(EXIT_FAILURE);
        }
        accept_whitespace();
        if ( accept_char('}') == 0 )
            break;
    } while( 1 );

    return section;
}

/**
 * Accepts a configuration file, which has the structure:
 * <name> { <section> ; ... }
 * @return The file that was decoded or NULL if it was not valid
 */
static cfg_file *accept_file( ) {
    cfg_section *section, *last_section = 0;
    cfg_file *file;
    const char *name;
    accept_whitespace();
    name = accept_name();
    if ( !name )
        return NULL;
    file = malloc( sizeof(cfg_file) );
    ensure_mem( file );
    accept_whitespace();
    expect_char( '{' );
    file->name = name;
    file->first_section = 0;
    do {
        section = accept_section();
        if ( !section ) {
            log( LOG_FATAL, "config", "Expected a valid cfg_section" );
            exit(EXIT_FAILURE);
        }
        if ( !last_section ) {
            file->first_section = section;
        } else {
            last_section->next = section;
        }
        last_section = section;
        accept_whitespace();
        if ( accept_char('}') == 0 )
            break;
        if ( accept_char(',') != 0 ) {
            log( LOG_FATAL, "config", "Expected a ," );
            exit(EXIT_FAILURE);
        }
    } while( 1 );

    file->last_section = last_section;

    return file;
}

/**
 * Loads a new configration file, if the file contains a section of type include,
 * it will load all config files referenced by it and insert their sections after the
 * include section.
 * @param path The path to the top level config file
 * @return The config file object
 */
cfg_file *load_config( const char *path ) {
    cfg_file *file;
    cfg_file *include;
    cfg_section *section;
    cfg_section *insert_point;
    cfg_entry *entry;

    parse_input = fopen(path, "r");
    parse_file = path;

    if (!parse_input) {
        log(LOG_FATAL, "config", "Could not open config file %s: %s", path, strerror(errno));
        exit(EXIT_FAILURE);
    }

    file = accept_file();
    if ( !file ) {
        log(LOG_FATAL, "config", "Could not parse config file %s: %s", path, strerror(errno));
        exit(EXIT_FAILURE);
    }

    fclose( parse_input );
    parse_input = NULL;

    for ( section = file->first_section; section; section = section->next ) {
        if ( strcmp(section->type, "include") != 0 )
            continue;
        insert_point = section;
        for ( entry = section->first_entry; entry; entry = entry->next ) {
            if ( entry->type != CONFIG_TYPE_STRING ) {
                log(LOG_WARN, "config", "Ignored non-string field %s in include section of file %s",
                        entry->name, file->name);
                continue;
            }
            include = load_config( entry->string );
            if ( include->first_section ) {
                insert_point->next = include->first_section;
                include->last_section = insert_point->next;
                insert_point = include->last_section;
            }
            free(include);
        }
    }

    return file;

}

/**
 * Looks up a section from a config file by name
 * @param file The file to search
 * @param name The section name to look for
 * @return The section object
 */
const cfg_section *cfg_find_section( const cfg_file *file, const char *name ) {
    cfg_section *cur;
    for (  cur = file->first_section; cur; cur = cur->next ) {
        if ( strcmp( cur->name, name ) == 0 )
            break;
    }
    return cur;
}

/**
 * Looks up an entry from a config section by name
 * @param section The section to search
 * @param name The entry name to look for
 * @return The entry object
 */
const cfg_entry *cfg_find_entry( const cfg_section *section, const char *name ) {
    cfg_entry *cur;
    for (  cur = section->first_entry; cur; cur = cur->next ) {
        if ( strcmp( cur->name, name ) == 0 )
            break;
    }
    return cur;
}

/**
 * Looks up a string entry from a config section by name
 * @param section The section to search
 * @param name The entry name to look for
 * @return The string value or NULL if not found
 */
const char *cfg_find_string( const cfg_section *section, const char *name ) {
    cfg_entry *cur;
    for (  cur = section->first_entry; cur; cur = cur->next ) {
        if ( strcmp( cur->name, name ) == 0 )
            break;
    }
    if ( !cur || cur->type != CONFIG_TYPE_STRING )
        return NULL;
    else
        return cur->string;
}

/**
 * Looks up a 64 bit integer entry from a config section by name
 * @param section The section to search
 * @param name The section name to look for
 * @param out  The integer value, by reference
 * @return Zero if successful or negative in the case of error
 */
int cfg_find_int64( const cfg_section *section, const char *name, uint64_t *out ) {
    cfg_entry *cur;
    for (  cur = section->first_entry; cur; cur = cur->next ) {
        if ( strcmp( cur->name, name ) == 0 )
            break;
    }
    if ( !cur || cur->type != CONFIG_TYPE_INT64 )
        return -1;
    *out = cur->int64;
    return 0;
}

/**
 * Looks up a 32 bit integer entry from a config section by name
 * @param section The section to search
 * @param name The section name to look for
 * @param out  The integer value, by reference
 * @return Zero if successful or negative in the case of error
 */
int cfg_find_int32( const cfg_section *section, const char *name, uint32_t *out ) {
    cfg_entry *cur;
    for (  cur = section->first_entry; cur; cur = cur->next ) {
        if ( strcmp( cur->name, name ) == 0 )
            break;
    }
    if ( !cur || cur->type != CONFIG_TYPE_INT64 )
        return -1;
    *out = cur->int64;
    return 0;
}

/**
 * Looks up a 16 bit integer entry from a config section by name
 * @param section The section to search
 * @param name The section name to look for
 * @param out  The integer value, by reference
 * @return Zero if successful or negative in the case of error
 */
int cfg_find_int16( const cfg_section *section, const char *name, uint16_t *out ) {
    cfg_entry *cur;
    for (  cur = section->first_entry; cur; cur = cur->next ) {
        if ( strcmp( cur->name, name ) == 0 )
            break;
    }
    if ( !cur || cur->type != CONFIG_TYPE_INT64 )
        return -1;
    *out = cur->int64;
    return 0;
}