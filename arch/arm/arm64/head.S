#include <arch/page.h>
#include <xtf/asm_macros.h>
#include <xen/xen.h>

/* 1 if BE, 0 if LE */
#define HEAD_FLAG_ENDIANNESS  0
#define HEAD_FLAG_PAGE_SIZE   ((PAGE_SHIFT - 10) / 2)
#define HEAD_FLAG_PHYS_BASE   1
#define HEAD_FLAGS            ((HEAD_FLAG_ENDIANNESS << 0) | \
                              (HEAD_FLAG_PAGE_SIZE << 1) |   \
                              (HEAD_FLAG_PHYS_BASE << 3))

/*
 * Print a string on the debug console
 *
 * Clobbers: x0, x1, x2, x3, x16
 */
#define PRINT(s)                                    \
    adr     x2, 98f;                                \
    mov     x1, #0;                                 \
97: ldrb    w3, [x2, x1];                           \
    add     x1, x1, #1;                             \
    cbnz    w3, 97b;                                \
    mov     x0, #CONSOLEIO_write;                   \
    mov     x16, #__HYPERVISOR_console_io;          \
    hvc     #XEN_HYPERCALL_TAG;                     \
.pushsection .rodata.str, "aMS", %progbits, 1;      \
98: .asciz  s;                                      \
.popsection

.section ".bss.page_aligned"
.p2align PAGE_SHIFT

.section ".text.head", "ax", @progbits
    b       _start                  /* branch to kernel start, magic */
    .long   0                       /* Executable code */
    .quad   0x0                     /* Image load offset from start of RAM */
    .quad   _end - _start           /* Effective Image size */
    .quad   HEAD_FLAGS              /* Informative flags, little-endian */
    .quad   0                       /* reserved */
    .quad   0                       /* reserved */
    .quad   0                       /* reserved */
    .byte   0x41                    /* Magic number, "ARM\x64" */
    .byte   0x52
    .byte   0x4d
    .byte   0x64
    .long   0                       /* reserved */


/* Load a physical address of \sym to \xb */
.macro load_paddr xb, sym
    ldr \xb, =\sym
    add \xb, \xb, x21
.endm

/*
 * Common register usage for assembly boot code
 *
 * x20 - DTB physical address (boot CPU only)
 * x21 - Offset between PA and VA ( PA - VA)
 * x30 - lr
 */
ENTRY(_start)
    /* Save DTB pointer */
    mov     x20, x0

    /* Calculate where we are */
    ldr     x22, =_start        /* x22 := vaddr(_start) */
    adr     x21, _start         /* x21 := paddr(_start) */
    sub     x21, x21, x22       /* x21 := phys-offset */

    PRINT("- XTF booting -\n")

    /* Start an infinite loop */
    PRINT("- Infinite loop -\n")
1:  b   1b
