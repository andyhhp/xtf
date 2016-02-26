#include <xtf/asm_macros.h>

#include <arch/x86/page.h>

        .data
        .p2align PAGE_SHIFT

/* PAE mappings of first 2M of memory in 4k pages. Uses 1x 4k page. */
GLOBAL(pae_l1_identmap)
        .long 0, 0 /* Unmap page at 0 to catch errors with NULL pointers. */
        .rept PAE_L1_PT_ENTRIES - 1
        .long (((. - pae_l1_identmap) / 8) << PAE_L1_PT_SHIFT) + \
              _PAGE_USER + _PAGE_RW + _PAGE_PRESENT
        .long 0
        .endr
SIZE(pae_l1_identmap)

/* PAE mappings up to 4G, mostly in 2M superpages. Uses 4x 4k pages. */
GLOBAL(pae_l2_identmap)
        .long pae_l1_identmap + _PAGE_USER + _PAGE_RW + _PAGE_PRESENT
        .long 0
        .rept (4 * PAE_L2_PT_ENTRIES) - 1
        .long (((. - pae_l2_identmap) / 8) << PAE_L2_PT_SHIFT) + \
              _PAGE_PSE + _PAGE_USER + _PAGE_RW + _PAGE_PRESENT
        .long 0
        .endr
SIZE(pae_l2_identmap)

/* PAE l3 pagetable.  Maps 4x l2 tables. */
GLOBAL(pae_l3_identmap)
        .rept 4
        .long pae_l2_identmap + (((. - pae_l3_identmap) / 8 ) << PAGE_SHIFT) + \
              _PAGE_USER + _PAGE_RW + _PAGE_PRESENT
        .long 0
        .endr
        .fill PAE_L3_PT_ENTRIES - 4, 8, 0
SIZE(pae_l3_identmap)

/* PAE l4 pagetable.  Maps 1x l3 table. */
GLOBAL(pae_l4_identmap)
        .long pae_l3_identmap + _PAGE_USER + _PAGE_RW + _PAGE_PRESENT
        .long 0
        .fill PAE_L4_PT_ENTRIES - 1, 8, 0
SIZE(pae_l4_identmap)

/* PAE l3 32bit quad.  Contains 4 64bit entries. */
GLOBAL(pae32_l3_identmap)
        .rept PAE32_L3_ENTRIES
        .long pae_l2_identmap + (((. - pae32_l3_identmap) / 8 ) << PAGE_SHIFT) + _PAGE_PRESENT
        .long 0
        .endr
SIZE(pae32_l3_identmap)

/*
 * Local variables:
 * tab-width: 8
 * indent-tabs-mode: nil
 * End:
 */
