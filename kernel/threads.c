//
// Created by pbx on 01/04/19.
//

#include "meloader.h"
#include <stdio.h>

me_mod *current_mod;

void krnl_set_current_mod(me_mod *mod) {
    current_mod = mod;
}

void start_thread( int n ) {
    fprintf(stderr,
            "[info] Starting thread %i: entry = %08x stack = %08x\n",
            n,
            current_mod->threads[n].entry_point,
            current_mod->threads[n].stack_top);
    switch_stack( current_mod->threads[n].entry_point,
                  current_mod->threads[n].stack_top );
}

int get_current_thread_id( void ) {
    void *stack;
    void *st_base, *st_top;
    int i;

    stack = get_esp();

    for ( i = 0; i < current_mod->num_threads; i++ ) {
        st_top = current_mod->threads[i].stack_top;
        st_base = st_top - current_mod->threads[i].stack_size;
        if ( stack >= st_base && stack <= st_top )
            return i;
    }

    fprintf( stderr, "[kernel] Unknown thread: Unrecognised stack address %p.",
             stack );

    return -1;

}
