#include <xtf/asm_macros.h>

#include <arch/x86/page.h>

        .data
        .p2align PAGE_SHIFT

/* Mapping of first 2M of memory in 4k pages. Uses 1x 4k page. */
GLOBAL(l1_identmap)
        .long 0, 0 /* Unmap page at 0 to catch errors with NULL pointers. */
        .rept L1_PT_ENTRIES - 1
        .long (((. - l1_identmap) / 8) << (PAGE_ORDER_4K + PAGE_SHIFT)) + \
              _PAGE_USER + _PAGE_RW + _PAGE_PRESENT
        .long 0
        .endr

/* Mappings up to 4G in 2M superpages. Uses 4x 4k pages. */
GLOBAL(l2_identmap)
        .long l1_identmap + _PAGE_USER + _PAGE_RW + _PAGE_PRESENT
        .long 0
        .rept (4 * L2_PT_ENTRIES) - 1
        .long (((. - l2_identmap) / 8) << (PAGE_ORDER_2M + PAGE_SHIFT)) + \
              _PAGE_PSE + _PAGE_USER + _PAGE_RW + _PAGE_PRESENT
        .long 0
        .endr
SIZE(l2_identmap)

/* l3 pagetable.  Maps 4x l2 tables. */
GLOBAL(l3_identmap)
        .rept 4
        .long l2_identmap + (((. - l3_identmap) / 8 ) << PAGE_SHIFT) + \
              _PAGE_USER + _PAGE_RW + _PAGE_PRESENT
        .long 0
        .endr
        .fill L3_PT_ENTRIES - 4, 8, 0
SIZE(l3_identmap)

/* l4 pagetable.  Maps 1x l3 table. */
GLOBAL(l4_identmap)
        .long l3_identmap + _PAGE_USER + _PAGE_RW + _PAGE_PRESENT
        .long 0
        .fill L4_PT_ENTRIES - 1, 8, 0
SIZE(l4_identmap)

/* l3 32bit PAE quad.  Contains 4 entries. */
GLOBAL(l3_paemap)
        .rept 4
        .long l2_identmap + (((. - l3_paemap) / 8 ) << PAGE_SHIFT) + _PAGE_PRESENT
        .long 0
        .endr
SIZE(l3_paemap)
