/**
 * @file arch/x86/include/arch/page-pae.h
 *
 * Definitions and helpers for PAE pagetable handling.
 */
#ifndef XTF_X86_PAGE_PAE_H
#define XTF_X86_PAGE_PAE_H

/** PAE pagetable entries are 64 bits wide. */
#define PAE_PTE_SIZE 8
/** PAE pagetable entries are 64 bits wide. */
#define PAE_PTE_ORDER 3

/** PAE pagetables encode 9 bits of index. */
#define PAE_PT_ORDER     9

/** @{ */
/** All PAE pagetables contain 512 entries. */
#define PAE_L1_PT_ENTRIES (PAGE_SIZE / PAE_PTE_SIZE)
#define PAE_L2_PT_ENTRIES (PAGE_SIZE / PAE_PTE_SIZE)
#define PAE_L3_PT_ENTRIES (PAGE_SIZE / PAE_PTE_SIZE)
#define PAE_L4_PT_ENTRIES (PAGE_SIZE / PAE_PTE_SIZE)

/* Other than PAE32_L3, which has 4 entries. */
#define PAE32_L3_ENTRIES 4
/** @} */

#define PAE_L1_PT_SHIFT 12
#define PAE_L2_PT_SHIFT 21
#define PAE_L3_PT_SHIFT 30
#define PAE_L4_PT_SHIFT 39

#ifndef __ASSEMBLY__

/** Integer representation of a PTE. */
typedef uint64_t pae_intpte_t;
#define PAE_PRIpte "016"PRIx64

static inline unsigned int pae_l1_table_offset(unsigned long va)
{
    return (va >> PAE_L1_PT_SHIFT) & (PAE_L1_PT_ENTRIES - 1);
}
static inline unsigned int pae_l2_table_offset(unsigned long va)
{
    return (va >> PAE_L2_PT_SHIFT) & (PAE_L2_PT_ENTRIES - 1);
}
static inline unsigned int pae_l3_table_offset(unsigned long va)
{
    return (va >> PAE_L3_PT_SHIFT) & (PAE_L3_PT_ENTRIES - 1);
}
#ifdef __x86_64__
static inline unsigned int pae_l4_table_offset(unsigned long va)
{
    return (va >> PAE_L4_PT_SHIFT) & (PAE_L4_PT_ENTRIES - 1);
}
#endif /* __x86_64__ */

#endif /* __ASSEMBLY__ */
#endif /* XTF_X86_PAGE_PAE_H */

/*
 * Local variables:
 * mode: C
 * c-file-style: "BSD"
 * c-basic-offset: 4
 * tab-width: 4
 * indent-tabs-mode: nil
 * End:
 */
