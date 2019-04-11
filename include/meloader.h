//
// Created by pbx on 18/03/19.
//

#ifndef MELOADER_H
#define MELOADER_H

#include <stdint.h>
#include <stdlib.h>

typedef struct {
    void *      stack_top;
    uintptr_t   stack_size;
    void *entry_point;
} me_thrd;

typedef struct {
    int         base;
    int         limit;
} me_seg;

typedef struct {
    int         mod_file;
    int         met_file;
    void       *met_map;
    off_t       met_size;
    void *bss_base;
    size_t bss_size;
    union {
        struct {
            void *text_base;
            size_t text_size;
            void *rodata_base;
            size_t rodata_size;
            void *heap_base;
            size_t heap_size;
            me_thrd *threads;
            int num_threads;
            me_seg *segments;
            int num_segments;
        };
        struct {
            void *load_base;
            size_t load_size;
            size_t context_size;
        };
    };
} me_mod;

typedef struct {
    void (*write)( int addr, const void *buffer, int count );
    void (*read )( int addr, void *buffer, int count );
} mmio_periph;

void write_hexn( int v );

void write_hex(int n,int i);

void write_str(char *s);

        void or_error( int cond, const char *fmt );

void insert_thunk( void *wr, void *target );

void *get_esp( void );

void switch_stack( void *entry, void *s_top );

void romlib_install_thunks();

extern me_mod *current_mod;

void krnl_write_seg( int seg, int offset, void *data, size_t count );
void krnl_read_seg ( int seg, int offset, void *data, size_t count );

#endif
