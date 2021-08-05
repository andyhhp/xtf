#include <arch/page.h>
#include <arch/mm.h>
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

/* Include console driver */
#ifdef CONFIG_PL011_EARLY_PRINTK
#include "pl011.inc"
#else
#include "hvc.inc"
#endif

/*
 * Print a string to the console
 * console_puts is a driver specific function to print a string.
 * Address of a string is stored in x0.
 */
#define PRINT(s)                                \
    adr   x0, 98f;                              \
    console_puts;                               \
.pushsection .rodata.str, "aMS", %progbits, 1;  \
98: .asciz s;                                   \
.popsection

.section ".bss.page_aligned"
.p2align PAGE_SHIFT

stack_start:
    .space STACK_SIZE
stack_end:

.text
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

    /* Disable MMU */
    bl      _mmu_disable

    /* Calculate where we are */
    ldr     x22, =_start        /* x22 := vaddr(_start) */
    adr     x21, _start         /* x21 := paddr(_start) */
    sub     x21, x21, x22       /* x21 := phys-offset */

    bl      console_init

    PRINT("- XTF booting -\n")

    PRINT("- Setup CPU -\n")
    bl      _cpu_setup

    /* Load the vector table */
    ldr     x2, =vector_table
    msr     vbar_el1, x2

#ifdef CONFIG_MMU
    /*
     * TTBR0_EL1 - identity mapping
     * TTBR1_EL1 - page tables
     */
    PRINT("- Setup page tables -\n")
    bl      _setup_page_tables
    bl      _setup_identity_mapping

    PRINT("- Enable MMU -\n")
    bl      _mmu_enable

    /* Jump to the runtime VA */
    ldr     x0, =mmu_enabled
    br      x0

mmu_enabled:
    bl      _setup_fixmap
#ifdef CONFIG_PL011_EARLY_PRINTK
    /* From now on use a virtual address to access the UART */
    ldr     x23, =PL011_EARLY_VA
#endif

    /*
     * Remove identity mapping.
     * By setting bit EPD0 we are disabling page table walk using TTBR0_EL1.
     */
    mrs     x0, tcr_el1
    add     x0, x0, #TCR_EPD0
    msr     tcr_el1, x0
#endif /* CONFIG_MMU */

    PRINT("- Zero BSS -\n")
    ldr     x0, =__start_bss
    ldr     x1, =__end_bss

    bl      flush_dcache_range
1:  str     xzr, [x0], #8
    cmp     x0, x1
    b.lo    1b

    /* Load BSS start address again as x0 has been modified in the upper loop */
    ldr     x0, =__start_bss
    bl      flush_dcache_range

    PRINT("- Setup stack -\n")
    ldr     x1, =stack_end
    mov     sp, x1

    /* Save boot arguments */
    ldr     x0, =boot_data
    stp     x21, x20, [x0]

    PRINT("- Jump to C world-\n")
    b       xtf_main
ENDFUNC(_start)

ENTRY(_mmu_disable)
    dsb     sy

    /* Turn off D-cache and MMU */
    mrs     x2, sctlr_el1
    bic     x2, x2, #SCTLR_M
    bic     x2, x2, #SCTLR_C
    msr     sctlr_el1, x2
    isb

    ret
ENDFUNC(_mmu_disable)

ENTRY(_cpu_setup)
    dsb     sy

    /* Set up memory attribute type tables */
    ldr     x0, =MAIRVAL
    msr     mair_el1, x0

    /* Set up TCR_EL1 register */
    ldr     x0, =TCRVAL
    mrs     x1, ID_AA64MMFR0_EL1
    /* Set TCR_EL1.IPS to ID_AA64MMFR0_EL1.PARange */
    bfi     x0, x1, #32, #3
    msr     tcr_el1, x0
    isb

    ret
ENDFUNC(_cpu_setup)

#ifdef CONFIG_MMU
ENTRY(_mmu_enable)
    dsb     sy

    /* Turn on D-cache and MMU */
    mrs     x2, sctlr_el1
    orr     x2, x2, #SCTLR_M
    orr     x2, x2, #SCTLR_C
    msr     sctlr_el1, x2
    isb

    ret
ENDFUNC(_mmu_enable)

ENTRY(_setup_page_tables)
    ldr          x0, =_text
    ldr          x1, =_end
    load_paddr   x3, l2_bpgtable
    load_paddr   x4, l1_bpgtable

    /* L1 table -> L2 table */
    /* Find page table index */
    lsr     x2, x0, #L1_TABLE_SHIFT
    and     x2, x2, #TABLE_ADDR_MASK

    /* Create page descriptor */
    ldr     x5, =DESC_PAGE_TABLE
    lsr     x6, x3, #PAGE_SHIFT
    orr     x5, x5, x6, lsl #PAGE_SHIFT

    /* Store the entry */
    str     x5, [x4, x2, lsl #3]

    /* Set TTBR1_EL1 */
    msr     ttbr1_el1, x4
    dsb     sy

    /* L2 table -> 2M blocks */
    /* Find page table index */
1:  lsr     x2, x0, #L2_TABLE_SHIFT
    and     x2, x2, #TABLE_ADDR_MASK

    /* Create block descriptor */
    add     x7, x0, x21
    lsr     x7, x7, #PAGE_SHIFT
    ldr     x5, =DESC_PAGE_BLOCK
    orr     x5, x5, x7, lsl #PAGE_SHIFT

    /* Store the entry */
    str     x5, [x3, x2, lsl #3]

    add     x0, x0, #L2_TABLE_SIZE
    cmp     x1, x0
    b.gt    1b

    ret
ENDFUNC(_setup_page_tables)

ENTRY(_setup_identity_mapping)
    load_paddr  x0, l1_idmap
    load_paddr  x1, _text

    /* Find the index */
    lsr     x2, x1, #L1_TABLE_SHIFT
    and     x2, x2, #TABLE_ADDR_MASK

    /* Create block descriptor */
    ldr     x3, =DESC_PAGE_BLOCK
    lsr     x4, x1, #PAGE_SHIFT
    orr     x3, x3, x4, lsl #PAGE_SHIFT

    /* Store the entry */
    str     x3, [x0, x2, lsl #3]

    /* Set TTBR0_EL1 */
    msr     ttbr0_el1, x0
    isb

    ret
ENDFUNC(_setup_identity_mapping)

ENTRY(_setup_fixmap)
    ldr            x0, =FIXMAP_ADDR(0)
    load_paddr     x1, fix_pgtable
    load_paddr     x2, l2_bpgtable

    /* L2 table -> L3 table(fixmap) */
    /* Find page table index */
    lsr     x3, x0, #L2_TABLE_SHIFT
    and     x3, x3, #TABLE_ADDR_MASK

    /* Create page descriptor */
    ldr     x4, =DESC_PAGE_TABLE
    lsr     x5, x1, #PAGE_SHIFT
    orr     x4, x4, x5, lsl #PAGE_SHIFT

    /* Store the entry */
    str     x4, [x2, x3, lsl #3]

#ifdef CONFIG_PL011_UART
    /* Map UART address in fixmap table */
    /* Find page table index */
    lsr     x3, x0, #L3_TABLE_SHIFT
    and     x3, x3, #TABLE_ADDR_MASK

    /* Create page descriptor */
    ldr     x5, =CONFIG_PL011_ADDRESS
    ldr     x4, =DESC_PAGE_TABLE_DEV
    lsr     x5, x5, #PAGE_SHIFT
    orr     x4, x4, x5, lsl #PAGE_SHIFT

    /* Store the entry */
    str     x4, [x1, x3, lsl #3]
#endif /* CONFIG_PL011_UART */

    dsb   nshst

    ret
ENDFUNC(_setup_fixmap)
#endif /* CONFIG_MMU */

ENTRY(console_init)
#ifdef CONFIG_PL011_EARLY_PRINTK
    ldr     x23, =CONFIG_PL011_ADDRESS
    pl011_uart_init x23, 0
    PRINT("- Early printk using PL011 UART -\n")
#else
    PRINT("- Early printk using Xen debug console -\n")
#endif
    ret
ENDFUNC(console_init)

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

/* Bad abort numbers */
#define BAD_SYNC    0
#define BAD_IRQ     1
#define BAD_FIQ     2
#define BAD_ERROR   3

.macro invalid, reason
    mov     x0, sp
    mov     x1, #\reason
    b       do_bad_mode
.endm

el0_error_invalid:
    invalid BAD_ERROR
ENDFUNC(el0_error_invalid)

el1_sync_invalid:
    invalid BAD_SYNC
ENDFUNC(el1_sync_invalid)

el1_irq_invalid:
    invalid BAD_IRQ
ENDFUNC(el1_irq_invalid)

el1_fiq_invalid:
    invalid BAD_FIQ
ENDFUNC(el1_fiq_invalid)

el1_error_invalid:
    invalid BAD_ERROR
ENDFUNC(el1_error_invalid)

/* SYNC exception handler */
    .align 6
el1_sync:
    entry_trap
    mov     x0, sp
    bl      do_trap_sync
    exit_trap
ENDFUNC(el1_sync)

/* IRQ exception handler */
    .align 6
el1_irq:
    entry_trap
    mov     x0, sp
    bl      do_trap_irq
    exit_trap
ENDFUNC(el1_irq)

/* Exception vectors */
.macro ventry label
    .align  7
    b       \label
.endm

    .align  11
ENTRY(vector_table)
    ventry el1_sync_invalid         /* Synchronous EL1t */
    ventry el1_irq_invalid          /* IRQ EL1t */
    ventry el1_fiq_invalid          /* FIQ EL1t */
    ventry el1_error_invalid        /* Error EL1t */

    ventry el1_sync                 /* Synchronous EL1h */
    ventry el1_irq                  /* IRQ EL1h */
    ventry el1_fiq_invalid          /* FIQ EL1h */
    ventry el1_error_invalid        /* Error EL1h */

    ventry el0_error_invalid        /* Synchronous 64-bit EL0 */
    ventry el0_error_invalid        /* IRQ 64-bit EL0 */
    ventry el0_error_invalid        /* FIQ 64-bit EL0 */
    ventry el0_error_invalid        /* Error 64-bit EL0 */

    ventry el0_error_invalid        /* Synchronous 32-bit EL0 */
    ventry el0_error_invalid        /* IRQ 32-bit EL0 */
    ventry el0_error_invalid        /* FIQ 32-bit EL0 */
    ventry el0_error_invalid        /* Error 32-bit EL0 */
ENDFUNC(vector_table)
