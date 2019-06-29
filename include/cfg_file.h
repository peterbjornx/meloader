//
// Created by pbx on 19/06/19.
//

#ifndef MELOADER_CFG_FILE_H
#define MELOADER_CFG_FILE_H

#define CONFIG_TYPE_STRING ('$')
#define CONFIG_TYPE_ENUM   ('%')
#define CONFIG_TYPE_INT64  ('&')
#include <stdint.h>

typedef struct cfg_file_s cfg_file;
typedef struct cfg_section_s cfg_section;
typedef struct cfg_entry_s cfg_entry;
typedef struct cfg_enum_s  cfg_enum;

struct cfg_file_s {
    const char  *name;
    cfg_section *first_section;
    cfg_section *last_section;
};

struct cfg_section_s {
    cfg_section *next;
    const char  *type;
    const char  *name;
    cfg_entry   *first_entry;
};

struct {
    const char *name;
    int         value;
} cfg_enum_s;

struct cfg_entry_s {
    cfg_entry  *next;
    const char *name;
    int         size;
    char        type;
    union {
        const cfg_enum *enumr;
        const char     *string;
        uint32_t        int32;
        uint64_t        int64;
    };
};

cfg_file *load_config( const char *path );
const cfg_section *cfg_find_section( const cfg_file *file, const char *name );
const cfg_entry *cfg_find_entry( const cfg_section *section, const char *name );
const char *cfg_find_string( const cfg_section *section, const char *name );
int cfg_find_int64( const cfg_section *section, const char *name, uint64_t *out );
int cfg_find_int32( const cfg_section *section, const char *name, uint32_t *out );

#endif //MELOADER_CFG_FILE_H
