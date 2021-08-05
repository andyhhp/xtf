#include <xtf/asm_macros.h>

#define ZIMAGE_MAGIC_NUMBER 0x016f2818

.arm

/*
 * Common register usage for assembly boot code
 *
 * r10 - DTB physical address (boot CPU only)
 * r9 - Offset between PA and VA ( PA - VA)
 */
ENTRY(_start)
    /* 8 NOPs that make the compressed kernel bootable on legacy ARM systems */
.rept 8
    mov     r0, r0
.endr
    b       startup
    /* Magic number used to identify this is an ARM Linux zImage */
    .word   ZIMAGE_MAGIC_NUMBER
    /* The address the zImage starts at (0 = relocatable) */
    .word   0
    /* The address the zImage ends at */
    .word   (_end - _start)
startup:
    /* Save DTB pointer */
    mov     r10, r2

    /* Calculate where we are */
    ldr     r0, =_start              /* r0 := vaddr(_start) */
    adr     r8, _start               /* r8 := paddr(_start) */
    sub     r9, r8, r0               /* r9 := phys-offset */

    /* Start an infinite loop */
1:  b       1b
