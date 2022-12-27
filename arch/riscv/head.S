#include <arch/page.h>

        .section ".text.head", "ax", %progbits

.globl _start
_start:
        /* sp = &stack[PAGE_SIZE] */
        la      sp, stack
        li      t0, PAGE_SIZE
        add     sp, sp, t0

        call    test_main

1:      j       1b

        .type   _start, %function
        .size   _start, . - _start

/*
 * Local variables:
 * tab-width: 8
 * indent-tabs-mode: nil
 * End:
 */
