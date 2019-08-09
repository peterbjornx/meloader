#include <stdint.h>
#include <log.h>
#include "user/meloader.h"

void insert_thunk( void *wr, void *target ) {
    uint8_t *opcode = wr;
    uint32_t *operand = wr + 1;
    *opcode = 0xE9;
    *operand = target - wr - 5;
}

void insert_thunk_rec( void *wr, void *target ) {
    uint8_t *opcode = wr;
    uint32_t *operand = wr + 1;
    logassert(*opcode == 0xE9, "romlib", "Could not place recursive thunk on non jmp");
    void * rtarg = *operand + wr + 5;
    insert_thunk( rtarg, target );
}


void *get_esp( void ) {
    void *esp;
    asm("mov %%esp, %0" : "=r" (esp));
    return esp;
}

void switch_stack( void *entry, void *s_top ) {
    asm("mov %1, %%esp\n"
        "jmp %0" :: "r" (entry), "r" (s_top));
}
