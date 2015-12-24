#ifndef XTF_X86_PAGE_H
#define XTF_X86_PAGE_H

#include <xtf/numbers.h>

/*
 * Nomenclature inherited from Xen.
 */

#define PAGE_SHIFT              12

#define PAGE_SIZE               (_AC(1, L) << PAGE_SHIFT)

#define PAGE_MASK               (~(PAGE_SIZE - 1))

#define PAGE_ORDER_4K           0
#define PAGE_ORDER_2M           9
#define PAGE_ORDER_1G           18

#define _PAGE_PRESENT           0x001
#define _PAGE_RW                0x002
#define _PAGE_USER              0x004
#define _PAGE_PSE               0x080

#define L1_PT_SHIFT             12
#define L2_PT_SHIFT             21
#define L3_PT_SHIFT             30
#define L4_PT_SHIFT             39

#define L1_PT_ENTRIES           512
#define L2_PT_ENTRIES           512
#define L3_PT_ENTRIES           512
#define L4_PT_ENTRIES           512

#ifndef __ASSEMBLY__

static inline unsigned int l1_table_offset(unsigned long va)
{ return (va >> L1_PT_SHIFT) & (L1_PT_ENTRIES - 1); }
static inline unsigned int l2_table_offset(unsigned long va)
{ return (va >> L2_PT_SHIFT) & (L2_PT_ENTRIES - 1); }
static inline unsigned int l3_table_offset(unsigned long va)
{ return (va >> L3_PT_SHIFT) & (L3_PT_ENTRIES - 1); }
#ifdef __x86_64__
static inline unsigned int l4_table_offset(unsigned long va)
{ return (va >> L4_PT_SHIFT) & (L4_PT_ENTRIES - 1); }
#endif /* __x86_64__ */


static inline uint64_t pte_to_paddr(uint64_t pte)
{ return pte & 0x000ffffffffff000ULL; }

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
