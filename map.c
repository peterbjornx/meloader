#include <sys/mman.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <romlib.h>
#include "meloader.h"
#include "manifest.h"

void krnl_set_current_mod(me_mod *mod);

off_t filesz(int fd ) {
    off_t oldpos, size;
    oldpos = lseek( fd, 0, SEEK_CUR );
    size = lseek( fd, 0, SEEK_END );
    lseek( fd, oldpos, SEEK_SET );
    return size;
}   

void or_error( int cond, const char *fmt ) {
    if ( cond )
        return;
    fprintf( stderr, fmt, strerror( errno ) );
    exit(EXIT_FAILURE);
}

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

    or_error( mod_attr != NULL, "Manifest did not contain mod_attr extension" );
    or_error( process != NULL,  "Manifest did not contain process extension" );
    or_error( threads != NULL, "Manifest did not contain threads extension" );
    or_error( mmios != NULL, "Manifest did not contain mmio extension" );

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
    fprintf(stderr, "[info] bss: %p size: %x ctxs: %x\n", mod->bss_base,
           process->bss_size, context_size);

}

void map_ranges_mod( me_mod *mod ) {
    void *map, *base;
    int i;

    fprintf(stderr, "[info]   text %p-%p\n", mod->text_base,
            mod->text_base+mod->text_size);

    map = mmap(
            (void *) mod->text_base,
            (size_t) mod->text_size + mod->rodata_size,
            PROT_READ | PROT_WRITE| PROT_EXEC,
            MAP_PRIVATE | MAP_FIXED,
            mod->mod_file,
            0);
    or_error(map == (void *) mod->text_base, "Could not map text: %s" );



    fprintf(stderr, "[info] rodata %p-%p\n", mod->rodata_base,
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


    fprintf(stderr, "[info]   heap %p-%p\n", mod->heap_base,
            mod->heap_base+mod->heap_size);
    map = mmap(
            (void *) mod->heap_base,
            (size_t) mod->heap_size,
            PROT_READ | PROT_WRITE,
            MAP_ANONYMOUS | MAP_PRIVATE | MAP_FIXED,
            -1,
            0);
    or_error(map == (void *) mod->heap_base, "Could not map heap: %s" );

    fprintf(stderr, "[info]    bss %p-%p\n", mod->bss_base,
            mod->bss_base+mod->bss_size);

    map = mmap(
            (void *) mod->bss_base,
            (size_t) mod->bss_size,
            PROT_READ | PROT_WRITE,
            MAP_ANONYMOUS | MAP_PRIVATE | MAP_FIXED,
            -1,
            0);
    or_error(map == (void *) mod->bss_base, "Could not map bss: %s" );

    for ( i = 0; i < mod->num_threads; i++ ){
        base = (void *) (mod->threads[i].stack_top - mod->threads[i].stack_size);
        map = mmap(
                base ,
                (size_t) mod->threads[i].stack_size,
                PROT_READ | PROT_WRITE,
                MAP_ANONYMOUS | MAP_PRIVATE | MAP_FIXED,
                -1,
                0);
        or_error( map == base, "Could not map stack: %s" );

    }
}

void populate_ranges_shlib(me_mod *mod) {
    man_ext_mod_attr *mod_attr;
    man_ext_shared_lib  *shlib;
    man_ext_locked_ranges *ranges;

    mod_attr = man_ext_find( mod->met_map, mod->met_size, 10 );
    shlib  = man_ext_find( mod->met_map, mod->met_size,  4 );
    ranges  = man_ext_find( mod->met_map, mod->met_size,  11 );

    or_error( mod_attr != NULL, "Manifest did not contain mod_attr extension" );
    or_error( shlib    != NULL, "Manifest did not contain shared lib extension" );
    or_error( ranges   != NULL, "Manifest did not contain locked ranges extension" );

    or_error(
            ranges->length >=
    sizeof(man_ext_locked_ranges) + sizeof(man_locked_range ),
            "Locked range extension is empty"
            );

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
    fprintf(stderr, "[info] Mapping file %i to %p-%p\n",mod->mod_file,
            mod->load_base, mod->load_base+mod->load_size);
    map = mmap(
            (void *) mod->load_base,
            (size_t) mod->load_size,
            PROT_READ | PROT_EXEC | PROT_WRITE,
            MAP_PRIVATE | MAP_FIXED,
            mod->mod_file,
            0);
    or_error(map == (void *) mod->load_base, "Could not map library: %s");

    if (mod->bss_size != 0) {
        map = mmap(
                (void *) mod->bss_base,
                (size_t) mod->bss_size,
                PROT_READ | PROT_WRITE,
                MAP_ANONYMOUS | MAP_PRIVATE | MAP_FIXED,
                -1,
                0);
        or_error(map == (void *) mod->bss_base, "Could not map library bss: %s");
    }

}

me_mod *open_mod( const char *modname, int nomet ) {
    char path[1024];
    me_mod *mod = malloc( sizeof(me_mod) );

    sprintf( path, "%s.mod", modname );
    mod->mod_file = open( path, O_RDONLY );
    or_error( mod->mod_file >= 0, "Could not open module file: %s\n" );

    if ( !nomet ) {
        sprintf(path, "%s.met", modname);
        mod->met_file = open(path, O_RDONLY);
        or_error(mod->met_file >= 0, "Could not open manifest file: %s\n");

        mod->met_size = filesz( mod->met_file );
        /* Module files are open */

        mod->met_map = mmap( NULL,
                             (size_t) mod->met_size,
                             PROT_READ,
                             MAP_PRIVATE,
                             mod->met_file,
                             0 );
        or_error( mod->met_map != MAP_FAILED, "Could not map manifest: %s\n"  );
    }

    return mod;
}

int main( int argc, char **argv ) {
    char *modname = argv[1];
    char *libname = argv[2];
    char *romname = argv[3];
    char *spiname = argv[4];
    uint32_t main_args[2];

    me_mod *mod = open_mod(modname, 0);
    me_mod *romlib = open_mod(romname, 1);
    me_mod *syslib = open_mod(libname, 0);

    printf("Testing getesp %p",get_esp());

    populate_ranges_romlib( romlib, 0x1000 );
    populate_ranges_shlib( syslib );
    populate_ranges_mod( mod, syslib->context_size );

    map_ranges_mod( mod );
    map_ranges_lib( syslib );
    map_ranges_lib( romlib );

    krnl_set_current_mod( mod );

    romlib_install_thunks();
    syslib_install_thunks();

    tracehub_fake_probe();
    tracehub_rs1_install();
    sks_install();
    spi_install();
    spi_openimg(spiname);
    hash_install();
    aes_install();

    main_args[0] = 0;
    main_args[1] = mod->threads[0].thread_id;

    uint32_t dfx_data = 0;
    uint64_t rom_bist = 1 << 11;
    uint8_t curr_keys[128];
    uint8_t prev_keys[128];
    snowball_add("bootpart", 0, 0, 5, 0, "FTPR");
    snowball_add("dfx_data", 0, 0, 4, 0, &dfx_data);
    snowball_add("rom_bist", 0, 0, 8, 0, &rom_bist);
    snowball_add("curr_keys", 0, 0, sizeof curr_keys, 0, &curr_keys);
    snowball_add("prev_keys", 0, 0, sizeof curr_keys, 0, &prev_keys);



    /* Below is for bup, for some reason it has its own read/write segment */
    insert_thunk((void *) 0x2D000, write_seg_32 );
    insert_thunk((void *) 0x2D016,  read_seg_32 );
    insert_thunk((void *) 0x2D026, write_seg_8  );
    insert_thunk((void *) 0x2D032,  read_seg_8  );

    start_thread(0, main_args, sizeof main_args);
    
    /* TEXT/RODATA | STACK + GUARD | HEAP | BSS | CTX */

}
