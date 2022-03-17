#include <arch/page.h>
#include <xtf/asm_macros.h>
#include <xen/xen.h>

#define ZIMAGE_MAGIC_NUMBER 0x016f2818

/*
 * Print a string on the debug console
 *
 * Clobbers: r0, r1, r2, r3, r12
 */
#define PRINT(s)                                    \
    ldr     r2, =98f;                               \
    add     r2, r2, r9;                             \
    mov     r1, #0;                                 \
97: ldrb    r3, [r2, r1];                           \
    add     r1, r1, #1;                             \
    cmp     r3, #0;                                 \
    bne     97b;                                    \
    mov     r0, #CONSOLEIO_write;                   \
    mov     r12, #__HYPERVISOR_console_io;          \
    __HVC(XEN_HYPERCALL_TAG);                       \
.pushsection .rodata.str, "aMS", %progbits, 1;      \
98: .asciz  s;                                      \
.popsection

.section ".text.head", "ax", %progbits
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

    PRINT("- XTF booting -\n")

    /* Start an infinite loop */
    PRINT("- Infinite loop -\n")
1:  b       1b
