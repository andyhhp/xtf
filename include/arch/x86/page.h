#ifndef XTF_X86_PAGE_H
#define XTF_X86_PAGE_H

#include <xtf/numbers.h>

/*
 * Nomenclature inherited from Xen.
 */

#define PAGE_SHIFT              12
#define PAGE_SIZE               (_AC(1, L) << PAGE_SHIFT)
#define PAGE_MASK               (~(PAGE_SIZE - 1))

#define PADDR_BITS              52
#define PADDR_MASK              ((_AC(1, ULL) << PADDR_BITS) - 1)

#include "page-pae.h"
#include "page-pse.h"

#define PAGE_ORDER_4K           0
#define PAGE_ORDER_2M           9
#define PAGE_ORDER_4M           10
#define PAGE_ORDER_1G           18

#define _PAGE_PRESENT           0x0001
#define _PAGE_RW                0x0002
#define _PAGE_USER              0x0004
#define _PAGE_PWT               0x0008
#define _PAGE_PCD               0x0010
#define _PAGE_ACCESSED          0x0020
#define _PAGE_DIRTY             0x0040
#define _PAGE_AD                (_PAGE_ACCESSED | _PAGE_DIRTY)
#define _PAGE_PSE               0x0080
#define _PAGE_PAT               0x0080
#define _PAGE_GLOBAL            0x0100
#define _PAGE_AVAIL             0x0e00
#define _PAGE_PSE_PAT           0x1000
#define _PAGE_NX                (_AC(1, ULL) << 63)

/* Shortened flags for use with PF_SYM(). */
#define _PAGE_P                 _PAGE_PRESENT
#define _PAGE_U                 _PAGE_USER
#define _PAGE_A                 _PAGE_ACCESSED
#define _PAGE_D                 _PAGE_DIRTY

#if CONFIG_PAGING_LEVELS == 2 /* PSE Paging */

#define PTE_SIZE  PSE_PTE_SIZE
#define PTE_ORDER PSE_PTE_ORDER

#define PT_ORDER PSE_PT_ORDER

#define L1_PT_SHIFT PSE_L1_PT_SHIFT
#define L2_PT_SHIFT PSE_L2_PT_SHIFT

#define L1_PT_ENTRIES PSE_L1_PT_ENTRIES
#define L2_PT_ENTRIES PSE_L2_PT_ENTRIES

#else /* CONFIG_PAGING_LEVELS == 2 */ /* PAE Paging */

#define PTE_SIZE  PAE_PTE_SIZE
#define PTE_ORDER PAE_PTE_ORDER

#define PT_ORDER PAE_PT_ORDER

#define L1_PT_SHIFT PAE_L1_PT_SHIFT
#define L2_PT_SHIFT PAE_L2_PT_SHIFT

#define L1_PT_ENTRIES PAE_L1_PT_ENTRIES
#define L2_PT_ENTRIES PAE_L2_PT_ENTRIES

#endif /* !CONFIG_PAGING_LEVELS == 2 */

#if CONFIG_PAGING_LEVELS >= 3 /* PAE Paging */

#define L3_PT_SHIFT PAE_L3_PT_SHIFT

#define L3_PT_ENTRIES PAE_L3_PT_ENTRIES

#endif /* CONFIG_PAGING_LEVELS >= 3 */

#if CONFIG_PAGING_LEVELS >= 4 /* PAE Paging */

#define L4_PT_SHIFT PAE_L4_PT_SHIFT

#define L4_PT_ENTRIES PAE_L4_PT_ENTRIES

#endif /* CONFIG_PAGING_LEVELS >= 4 */


#ifndef __ASSEMBLY__

/*
 * Always consider "physical" addresses to be 64bits wide, even in 32bit mode.
 */
typedef uint64_t paddr_t;
#define PRIpaddr "016"PRIx64

#if CONFIG_PAGING_LEVELS > 0 /* Some form of pagetables. */

#if CONFIG_PAGING_LEVELS == 2 /* PSE Paging */

typedef pse_intpte_t intpte_t;
#define PRIpte PSE_PRIpte

static inline unsigned int l1_table_offset(unsigned long va)
{
    return pse_l1_table_offset(va);
}
static inline unsigned int l2_table_offset(unsigned long va)
{
    return pse_l2_table_offset(va);
}

#else /* CONFIG_PAGING_LEVELS == 2 */ /* PAE Paging */

typedef pae_intpte_t intpte_t;
#define PRIpte PAE_PRIpte

static inline unsigned int l1_table_offset(unsigned long va)
{
    return pae_l1_table_offset(va);
}
static inline unsigned int l2_table_offset(unsigned long va)
{
    return pae_l2_table_offset(va);
}

#endif /* !CONFIG_PAGING_LEVELS == 2 */

#if CONFIG_PAGING_LEVELS >= 3 /* PAE Paging */

static inline unsigned int l3_table_offset(unsigned long va)
{
    return pae_l3_table_offset(va);
}

#endif /* CONFIG_PAGING_LEVELS >= 3 */

#if CONFIG_PAGING_LEVELS >= 4 /* PAE Paging */

static inline unsigned int l4_table_offset(unsigned long va)
{
    return pae_l4_table_offset(va);
}

#endif /* CONFIG_PAGING_LEVELS >= 4 */

#else /* CONFIG_PAGING_LEVELS > 0 */

/* Enough compatibility to compile in unpaged environments. */
typedef unsigned long intpte_t;
#define PRIpte "08lx"

#endif

#ifdef CONFIG_HVM

extern pae_intpte_t pae_l1_identmap[PAE_L1_PT_ENTRIES];
extern pae_intpte_t pae_l2_identmap[4 * PAE_L2_PT_ENTRIES];
extern pae_intpte_t pae_l3_identmap[PAE_L3_PT_ENTRIES];
extern pae_intpte_t pae_l4_identmap[PAE_L4_PT_ENTRIES];
extern pae_intpte_t pae32_l3_identmap[PAE32_L3_ENTRIES];

extern pse_intpte_t pse_l1_identmap[PSE_L1_PT_ENTRIES];
extern pse_intpte_t pse_l2_identmap[PSE_L2_PT_ENTRIES];

/* Aliases of the live tables (PAE or PSE as appropriate). */
extern intpte_t l1_identmap[L1_PT_ENTRIES];
#if CONFIG_PAGING_LEVELS >= 3
extern intpte_t l2_identmap[4 *L2_PT_ENTRIES];
#else
extern intpte_t l2_identmap[L2_PT_ENTRIES];
#endif

#endif /* CONFIG_HVM */

#endif /* !__ASSEMBLY__ */

#endif /* XTF_X86_PAGE_H */

/*
 * Local variables:
 * mode: C
 * c-file-style: "BSD"
 * c-basic-offset: 4
 * tab-width: 4
 * indent-tabs-mode: nil
 * End:
 */
