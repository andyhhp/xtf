#include <xtf/asm_macros.h>

#include <arch/x86/page.h>

#define PAGETABLE_START(sym)   \
        GLOBAL(sym)

#define PAGETABLE_END(sym)     \
        .type sym, STT_OBJECT; \
        SIZE(sym)

#define _PAGE_LEAF    (_PAGE_AD + _PAGE_RW + _PAGE_PRESENT)
#define _PAGE_SUPER   (_PAGE_PSE + _PAGE_LEAF)
#define _PAGE_NONLEAF (_PAGE_USER + _PAGE_LEAF)

#define PAE_IDX(sym) ((. - (sym)) / PAE_PTE_SIZE)
#define PSE_IDX(sym) ((. - (sym)) / PSE_PTE_SIZE)

        .data
        .p2align PAGE_SHIFT

/* PAE mappings of first 2M of memory in 4k pages. Uses 1x 4k page. */
PAGETABLE_START(pae_l1_identmap)
        .long 0, 0 /* Unmap page at 0 to catch errors with NULL pointers. */
        .rept PAE_L1_PT_ENTRIES - 1
        .long (PAE_IDX(pae_l1_identmap) << PAE_L1_PT_SHIFT) + _PAGE_LEAF
        .long 0
        .endr
PAGETABLE_END(pae_l1_identmap)

/* PAE mappings up to 4G, mostly in 2M superpages. Uses 4x 4k pages. */
PAGETABLE_START(pae_l2_identmap)
        .long pae_l1_identmap + _PAGE_NONLEAF
        .long 0
        .rept (4 * PAE_L2_PT_ENTRIES) - 1
        .long (PAE_IDX(pae_l2_identmap) << PAE_L2_PT_SHIFT) + _PAGE_SUPER
        .long 0
        .endr
PAGETABLE_END(pae_l2_identmap)
.Lpae_l2_identmap_end:

/* PAE l3 pagetable.  Maps 4x l2 tables. */
PAGETABLE_START(pae_l3_identmap)
        .rept 4
        .long pae_l2_identmap + (PAE_IDX(pae_l3_identmap) << PAGE_SHIFT) + _PAGE_NONLEAF
        .long 0
        .endr
        .fill PAE_L3_PT_ENTRIES - 4, 8, 0
PAGETABLE_END(pae_l3_identmap)

/* PAE l4 pagetable.  Maps 1x l3 table. */
PAGETABLE_START(pae_l4_identmap)
        .long pae_l3_identmap + _PAGE_NONLEAF
        .long 0
        .fill PAE_L4_PT_ENTRIES - 1, 8, 0
PAGETABLE_END(pae_l4_identmap)

/* PSE mappings of the first 4M of memory in 4k pages.  Uses 1x 4k page. */
PAGETABLE_START(pse_l1_identmap)
        .long 0 /* Unmap page at 0 to catch errors with NULL pointers. */
        .rept PSE_L1_PT_ENTRIES - 1
        .long (PSE_IDX(pse_l1_identmap) << PSE_L1_PT_SHIFT) + _PAGE_LEAF
        .endr
PAGETABLE_END(pse_l1_identmap)

/* PSE mappings up to 4G, mostly in 4M superpages.  Uses 1x 4k page. */
PAGETABLE_START(pse_l2_identmap)
        .long pse_l1_identmap + _PAGE_NONLEAF
        .rept PSE_L2_PT_ENTRIES - 1
        .long (PSE_IDX(pse_l2_identmap) << PSE_L2_PT_SHIFT) + _PAGE_SUPER
        .endr
PAGETABLE_END(pse_l2_identmap)
.Lpse_l2_identmap_end:

/* PAE l3 32bit quad.  Contains 4 64bit entries. */
PAGETABLE_START(pae32_l3_identmap)
        .rept PAE32_L3_ENTRIES
        .long pae_l2_identmap + (PAE_IDX(pae32_l3_identmap) << PAGE_SHIFT) + _PAGE_PRESENT
        .long 0
        .endr
PAGETABLE_END(pae32_l3_identmap)


        /* Aliases of the live tables (PAE or PSE as appropriate). */
#if CONFIG_PAGING_LEVELS >= 3
        .set l1_identmap, pae_l1_identmap
        .set l2_identmap, pae_l2_identmap
        .set l2_identmap_end, .Lpae_l2_identmap_end
#else
        .set l1_identmap, pse_l1_identmap
        .set l2_identmap, pse_l2_identmap
        .set l2_identmap_end, .Lpse_l2_identmap_end
#endif
        .global l1_identmap
        .global l2_identmap
        .global l2_identmap_end

        /* Alias of the pagetable %cr3 points at. */
#if CONFIG_PAGING_LEVELS > 0

#if CONFIG_PAGING_LEVELS == 2
        .set cr3_target, pse_l2_identmap
#elif CONFIG_PAGING_LEVELS == 3
        .set cr3_target, pae32_l3_identmap
#elif CONFIG_PAGING_LEVELS == 4
        .set cr3_target, pae_l4_identmap
#else
# error Bad paging mode
#endif

#else
        .set cr3_target, 0
#endif
        .global cr3_target

/*
 * Local variables:
 * tab-width: 8
 * indent-tabs-mode: nil
 * End:
 */
