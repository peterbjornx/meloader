#include <sys/mman.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <user/romlib.h>
#include <log.h>
#include "user/meloader.h"
#include "manifest.h"
#include "file.h"

void populate_ranges_mod( me_mod *mod, size_t context_size ) {
    man_ext_mod_attr *mod_attr;
    man_ext_threads  *threads;
    man_ext_process  *process;
    man_ext_mmio_ranges *mmios;
    int i;
    void *next_base;
    uint32_t t;

    mod_attr = man_ext_find( mod->met_map, mod->met_size, 10 );
    threads  = man_ext_find( mod->met_map, mod->met_size,  6 );
    process  = man_ext_find( mod->met_map, mod->met_size,  5 );
    mmios    = man_ext_find( mod->met_map, mod->met_size,  8 );

    logassert( mod_attr != NULL, "loader", "Manifest did not contain mod_attr extension" );
    logassert( process != NULL, "loader",  "Manifest did not contain process extension" );
    logassert( threads != NULL, "loader", "Manifest did not contain threads extension" );
    logassert( mmios != NULL, "loader", "Manifest did not contain mmio extension" );

    mod->text_base = (void *) process->priv_code_base_address;
    mod->text_size = process->uncompressed_priv_code_size;
    mod->rodata_base = mod->text_base + mod->text_size;
    mod->rodata_size = mod_attr->uncompressed_size - mod->text_size;
    next_base = mod->rodata_base + mod->rodata_size;

    mod->num_threads = (threads->length - sizeof(man_ext)) / sizeof(man_thread);
    mod->threads = calloc((size_t) mod->num_threads, sizeof(me_thrd) );

    t = process->main_thread_id;

    for ( i = 0; i < mod->num_threads; i++ ) {
        mod->threads[i].stack_size = threads->threads[i].stack_size;
        next_base += 0x1000; /* Guard page below stack ? */
        next_base += mod->threads[i].stack_size;
        mod->threads[i].stack_top = next_base;
        mod->threads[i].thread_id = t++;
    }
    mod->threads[0].entry_point = (void *) process->main_thread_entry;

    mod->num_segments = (mmios->length - sizeof(man_ext)) / sizeof(man_mmio_range);
    mod->segments     = calloc((size_t) mod->num_segments, sizeof(me_seg) );
    for ( i = 0; i < mod->num_segments; i++ ) {
        mod->segments[i].base = mmios->mmio_range_defs[i].base;
        mod->segments[i].limit = mmios->mmio_range_defs[i].limit;
    }
    //TODO: Map other segments as well?

    mod->heap_base = next_base;
    mod->heap_size = process->default_heap_size;
    next_base += process->default_heap_size;
    mod->bss_base = next_base;
    mod->bss_size = process->bss_size + context_size;

    log(LOG_DEBUG, "loader", "bss: %p size: %x ctxs: %x", mod->bss_base,
           process->bss_size, context_size);

}

void map_ranges_mod( me_mod *mod ) {
    void *map, *base;
    int i;

    log(LOG_DEBUG, "loader", "text %p-%p", mod->text_base,
            mod->text_base+mod->text_size);

    map = mmap(
            (void *) mod->text_base,
            (size_t) mod->text_size + mod->rodata_size,
            PROT_READ | PROT_WRITE| PROT_EXEC,
            MAP_PRIVATE | MAP_FIXED,
            mod->mod_file,
            0);
    logassert(map == (void *) mod->text_base,
            "loader", "Could not map text: %s", strerror( errno ) );



    log(LOG_DEBUG, "loader", "rodata %p-%p", mod->rodata_base,
            mod->rodata_base+mod->rodata_size);

    /*
    map = mmap(
            (void *) mod->rodata_base,
            (size_t) mod->rodata_size,
            PROT_READ,
            MAP_PRIVATE | MAP_FIXED,
            mod->mod_file,
            (off_t)  mod->text_size);
    or_error(map == (void *) mod->rodata_base, "Could not map rodata: %s" );
    */


    log(LOG_DEBUG, "loader", "heap %p-%p", mod->heap_base,
            mod->heap_base+mod->heap_size);
    map = mmap(
            (void *) mod->heap_base,
            (size_t) mod->heap_size,
            PROT_READ | PROT_WRITE,
            MAP_ANONYMOUS | MAP_PRIVATE | MAP_FIXED,
            -1,
            0);
    logassert(map == (void *) mod->heap_base,
            "loader", "Could not map heap: %s", strerror( errno ) );

    log(LOG_DEBUG, "loader", "bss %p-%p", mod->bss_base,
            mod->bss_base+mod->bss_size);

    map = mmap(
            (void *) mod->bss_base,
            (size_t) mod->bss_size,
            PROT_READ | PROT_WRITE,
            MAP_ANONYMOUS | MAP_PRIVATE | MAP_FIXED,
            -1,
            0);
    logassert(map == (void *) mod->bss_base, "loader",
            "Could not map bss: %s", strerror( errno ) );

    for ( i = 0; i < mod->num_threads; i++ ){
        base = (void *) (mod->threads[i].stack_top - mod->threads[i].stack_size);
        map = mmap(
                base ,
                (size_t) mod->threads[i].stack_size,
                PROT_READ | PROT_WRITE,
                MAP_ANONYMOUS | MAP_PRIVATE | MAP_FIXED,
                -1,
                0);
        logassert( map == base, "loader",
                "Could not map stack: %s", strerror( errno ) );

    }
}

void populate_ranges_shlib(me_mod *mod) {
    man_ext_mod_attr *mod_attr;
    man_ext_shared_lib  *shlib;
    man_ext_locked_ranges *ranges;

    mod_attr = man_ext_find( mod->met_map, mod->met_size, 10 );
    shlib  = man_ext_find( mod->met_map, mod->met_size,  4 );
    ranges  = man_ext_find( mod->met_map, mod->met_size,  11 );

    logassert( mod_attr != NULL, "loader", "Manifest did not contain mod_attr extension" );
    logassert( shlib    != NULL, "loader", "Manifest did not contain shared lib extension" );
    logassert( ranges   != NULL, "loader", "Manifest did not contain locked ranges extension" );

    logassert(
            ranges->length >=
    sizeof(man_ext_locked_ranges) + sizeof(man_locked_range ),
            "loader", "Locked range extension is empty" );

    mod->load_base = ( void * ) ranges->ranges[0].base;
    mod->load_size = mod_attr->uncompressed_size;
    mod->bss_base = mod->load_base + mod->load_size;
    mod->bss_size = shlib->total_alloc_virtual_space - mod->load_size;
    mod->context_size = shlib->context_size;

}

void populate_ranges_romlib( me_mod *mod, uintptr_t base ) {
    mod->load_base = ( void * ) base;
    mod->load_size = (size_t) filesz(mod->mod_file );;
    mod->bss_base = mod->load_base + mod->load_size;
    mod->bss_size = 0;
    mod->context_size = 0;
}

void map_ranges_lib( me_mod *mod ) {
    void *map;
    log(LOG_DEBUG, "loader", "Mapping file %i to %p-%p",mod->mod_file,
            mod->load_base, mod->load_base+mod->load_size);
    map = mmap(
            (void *) mod->load_base,
            (size_t) mod->load_size,
            PROT_READ | PROT_EXEC | PROT_WRITE,
            MAP_PRIVATE | MAP_FIXED,
            mod->mod_file,
            0);
    logassert(map == (void *) mod->load_base,
            "loader", "Could not map library: %s", strerror( errno ) );

    if (mod->bss_size != 0) {
        map = mmap(
                (void *) mod->bss_base,
                (size_t) mod->bss_size,
                PROT_READ | PROT_WRITE,
                MAP_ANONYMOUS | MAP_PRIVATE | MAP_FIXED,
                -1,
                0);
        logassert(map == (void *) mod->bss_base,
                "loader", "Could not map library bss: %s", strerror( errno ) );
    }

}

me_mod *open_mod( const char *modname, int nomet ) {
    char path[1024];
    me_mod *mod = malloc( sizeof(me_mod) );

    sprintf( path, "%s.mod", modname );
    mod->mod_file = open( path, O_RDONLY );
    logassert( mod->mod_file >= 0, "loader", "Could not open module file: %s", path );

    if ( !nomet ) {
        sprintf(path, "%s.met", modname);
        mod->met_file = open(path, O_RDONLY);
        logassert(mod->met_file >= 0, "loader", "Could not open manifest file: %s");

        mod->met_size = filesz( mod->met_file );
        /* Module files are open */

        mod->met_map = mmap( NULL,
                             (size_t) mod->met_size,
                             PROT_READ,
                             MAP_PRIVATE,
                             mod->met_file,
                             0 );
        logassert( mod->met_map != MAP_FAILED, "loader", "Could not map manifest: %s"  );
    }

    return mod;
}