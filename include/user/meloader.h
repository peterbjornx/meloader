//
// Created by pbx on 18/03/19.
//

#ifndef MELOADER_H
#define MELOADER_H

#include <stdint.h>
#include <stdlib.h>
#include "devreg.h"

typedef struct {
    uint32_t    thread_id;
    void *      stack_top;
    uintptr_t   stack_size;
    void *      entry_point;
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

void *get_esp( void );

void switch_stack( void *entry, void *s_top );
void start_thread( int n, void *args, size_t args_size );
void * get_thread_tls( int i );
int get_current_thread_id( void );
void syslib_install_thunks();

void krnl_set_current_mod(me_mod *mod);
extern me_mod *current_mod;

void krnl_write_seg( int seg, int offset, const void *data, size_t count );
void krnl_read_seg ( int seg, int offset, void *data, size_t count );
void krnl_set_cpu( device_instance *_cpu );

void dma_write( int address, const void *data, size_t count );
void dma_read ( int address, void *data, size_t count );

int snowball_read( void *par );
void snowball_add(const char *name, int unk0, int flags, int size, int unk1,
                  void *data);
#endif
