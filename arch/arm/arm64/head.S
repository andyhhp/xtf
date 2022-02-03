        .section ".text.head", "ax", %progbits

#define ZIMAGE64_MAGIC                  0x644d5241

#define ZIMAGE64_FLAGS_LE               (0 << 0)
#define ZIMAGE64_FLAGS_PG_UNK           (0 << 1)
#define ZIMAGE64_FLAGS_PHYS_ANY         (1 << 3)

#define ZIMAGE64_FLAGS \
        (ZIMAGE64_FLAGS_LE | ZIMAGE64_FLAGS_PG_UNK | ZIMAGE64_FLAGS_PHYS_ANY)

        /*
         * zImage64 header:
         *   See https://www.kernel.org/doc/html/latest/arm64/booting.html
         */
.globl _start
_start:
        b       startup                 /* code0 */
        nop                             /* code1 */
        .quad   0                       /* text_offset:  0 -> relocatable */
        .quad   _end - _start           /* size */
        .quad   ZIMAGE64_FLAGS          /* flags */
        .quad   0                       /* reserved */
        .quad   0                       /* reserved */
        .quad   0                       /* reserved */
        .long   ZIMAGE64_MAGIC          /* magic */
        .long   0                       /* reserved */

startup:
        /* Calculate x21 = phys-offset */
        ldr     x22, =_start
        adr     x21, _start
        sub     x21, x21, x22

        /* sp = &stack[4096] + phys-offset */
        ldr     x0, =stack + 4096
        add     sp, x0, x21

        bl      test_main

1:      b       1b

        .type   startup, %function
        .size   startup, . - startup

/*
 * Local variables:
 * tab-width: 8
 * indent-tabs-mode: nil
 * End:
 */
