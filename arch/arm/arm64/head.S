#include <arch/page.h>
#include <xtf/asm_macros.h>
#include <xen/xen.h>

/* Necessary for older compilers */
lr .req x30

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

stack_start:
    .space STACK_SIZE
stack_end:

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
    /* Disable all IRQs */
    msr     daifset, #0xf

    /* Save DTB pointer */
    mov     x20, x0

    /*
     * Turn off D-cache.
     * No need to disable MMU. Image protocol mandates that MMU must be off
     * when entering the kernel.
     */
    dsb     sy
    mrs     x2, sctlr_el1
    bic     x2, x2, #SCTLR_C
    msr     sctlr_el1, x2
    isb

    /* Calculate where we are */
    ldr     x22, =_start        /* x22 := vaddr(_start) */
    adr     x21, _start         /* x21 := paddr(_start) */
    sub     x21, x21, x22       /* x21 := phys-offset */

    PRINT("- XTF booting -\n")

    PRINT("- Setup CPU -\n")
    bl      cpu_setup

    /* Load the vector table */
    ldr     x2, =vector_table
    msr     vbar_el1, x2

    PRINT("- Zero BSS -\n")
    load_paddr  x0, __start_bss
    load_paddr  x1, __end_bss

    /*
     * The BSS is not going to be part of the loaded image, so there is no
     * guarantee in the state of the cache. Therefore we need to clean and
     * invalidate the cache for BSS region before and after zeroing BSS.
     */
    bl      clean_and_invalidate_dcache
1:  str     xzr, [x0], #8
    cmp     x0, x1
    b.lo    1b

    /* Load BSS start address again as x0 has been modified in the upper loop */
    load_paddr  x0, __start_bss
    bl      clean_and_invalidate_dcache

    PRINT("- Setup stack -\n")
    ldr     x1, =stack_end
    mov     sp, x1

    /* Save boot arguments */
    ldr     x0, =boot_data
    stp     x21, x20, [x0]

    /* Start an infinite loop */
    PRINT("- Infinite loop -\n")
2:  b   2b
ENDFUNC(_start)

/*
 * Setup CPU for enabling MMU.
 *
 * Clobbers: x0
 */
cpu_setup:
    dsb     sy

    /* Set up memory attribute type tables */
    ldr     x0, =MAIRVAL
    msr     mair_el1, x0
    isb

    ret
ENDFUNC(cpu_setup)

/* Save state */
.macro entry_trap
    sub     sp, sp, #(272 - 240)     /* offset: spsr_el1 - lr */
    stp     x28, x29, [sp, #-16]!
    stp     x26, x27, [sp, #-16]!
    stp     x24, x25, [sp, #-16]!
    stp     x22, x23, [sp, #-16]!
    stp     x20, x21, [sp, #-16]!
    stp     x18, x19, [sp, #-16]!
    stp     x16, x17, [sp, #-16]!
    stp     x14, x15, [sp, #-16]!
    stp     x12, x13, [sp, #-16]!
    stp     x10, x11, [sp, #-16]!
    stp     x8, x9, [sp, #-16]!
    stp     x6, x7, [sp, #-16]!
    stp     x4, x5, [sp, #-16]!
    stp     x2, x3, [sp, #-16]!
    stp     x0, x1, [sp, #-16]!

    add     x21, sp, #272           /* offset: spsr_el1 */
    stp     lr, x21, [sp, #240]     /* offset: lr */
    mrs     x21, elr_el1
    mrs     x22, spsr_el1
    stp     x21, x22, [sp, #256]    /* offset: pc */
.endm

/* Restore state */
.macro exit_trap
    ldp     x21, x22, [sp, #256]    /* offset: pc */
    ldp     x0, x1, [sp], #16
    ldp     x2, x3, [sp], #16
    ldp     x4, x5, [sp], #16
    ldp     x6, x7, [sp], #16
    ldp     x8, x9, [sp], #16

    msr     elr_el1, x21            /* set up the return data */
    msr     spsr_el1, x22

    ldp     x10, x11, [sp], #16
    ldp     x12, x13, [sp], #16
    ldp     x14, x15, [sp], #16
    ldp     x16, x17, [sp], #16
    ldp     x18, x19, [sp], #16
    ldp     x20, x21, [sp], #16
    ldp     x22, x23, [sp], #16
    ldp     x24, x25, [sp], #16
    ldp     x26, x27, [sp], #16
    ldp     x28, x29, [sp], #16

    ldr     lr, [sp], #32           /* offset: spsr_el1 - lr */
    eret
.endm

/*
 * Invalid vector entry trap handler.
 *
 * Clobbers: x0
 */
invalid_vector_entry:
    mov     x0, sp
    b       do_bad_mode
ENDFUNC(invalid_vector_entry)

    .align 6
/*
 * SYNC exception handler.
 *
 * Clobbers: x0
 */
el1_sync:
    entry_trap
    mov     x0, sp
    bl      do_trap_sync
    exit_trap
ENDFUNC(el1_sync)

    .align 6
/*
 * IRQ exception handler.
 *
 * Clobbers: x0
 */
el1_irq:
    entry_trap
    mov     x0, sp
    bl      do_trap_irq
    exit_trap
ENDFUNC(el1_irq)

.macro ventry label
    .align  7
    b       \label
.endm

    .align  11

/* Vector table for exceptions. */
vector_table:
    ventry invalid_vector_entry        /* Synchronous EL1t */
    ventry invalid_vector_entry        /* IRQ EL1t */
    ventry invalid_vector_entry        /* FIQ EL1t */
    ventry invalid_vector_entry        /* Error EL1t */

    ventry el1_sync                    /* Synchronous EL1h */
    ventry el1_irq                     /* IRQ EL1h */
    ventry invalid_vector_entry        /* FIQ EL1h */
    ventry invalid_vector_entry        /* Error EL1h */

    ventry invalid_vector_entry        /* Synchronous 64-bit EL0 */
    ventry invalid_vector_entry        /* IRQ 64-bit EL0 */
    ventry invalid_vector_entry        /* FIQ 64-bit EL0 */
    ventry invalid_vector_entry        /* Error 64-bit EL0 */

    ventry invalid_vector_entry        /* Synchronous 32-bit EL0 */
    ventry invalid_vector_entry        /* IRQ 32-bit EL0 */
    ventry invalid_vector_entry        /* FIQ 32-bit EL0 */
    ventry invalid_vector_entry        /* Error 32-bit EL0 */
ENDFUNC(vector_table)
