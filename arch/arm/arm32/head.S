        .section ".text.head", "ax", %progbits
        .code 32

#define ZIMAGE32_MAGIC                  0x016f2818
#define ZIMAGE32_LE                     0x04030201

        /*
         * The Linux zImage32 header has no offical documentation.
         *
         * It is formed as:
         *
         *   u32 nops[8];         // Old a.out header
         *   u32 code0;           // Branch to start logic
         *   u32 zimage32_magic;  // 0x016f2818
         *   u32 start;           // Start in RAM, or 0 for relocatable
         *   u32 end;             // End in RAM, or size for relocatable
         *   u32 endian_magic;    // 0x04030201 for LE, 0x01020304 ro BE
         *   u32 kexec_magic;     // 0x45454545 (if kexec_addr present)
         *   u32 kexec_addr;
         *   ...                  // Future extentions
         */
.globl _start
_start:

        .rept 8
                nop
        .endr

        b       startup                 /* code0 */
        .word   ZIMAGE32_MAGIC          /* zimage32_magic */
        .word   0                       /* start: 0 -> relocatable */
        .word   _end - _start           /* end: (size) */
        .word   ZIMAGE32_LE             /* endian_magic */

startup:
        /* Calculate r9 = phys-offset */
        ldr     r0, =_start
        adr     r8, _start
        sub     r9, r8, r0

        /* sp = &stack[4096] + phys-offset */
        ldr     sp, =stack + 4096
        add     sp, sp, r9

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
