/**
 * @file include/arch/x86/page-pse.h
 *
 * Definitions and helpers for PSE pagetable handling.
 */
#ifndef XTF_X86_PAGE_PSE_H
#define XTF_X86_PAGE_PSE_H

/** PSE pagetable entries are 32 bits wide. */
#define PSE_PTE_SZ 4

/** @{ */
/** All PSE pagetables contain 1024 entries. */
#define PSE_L1_PT_ENTRIES (PAGE_SIZE / PSE_PTE_SZ)
#define PSE_L2_PT_ENTRIES (PAGE_SIZE / PSE_PTE_SZ)
/** @} */

#define PSE_L1_PT_SHIFT 12
#define PSE_L2_PT_SHIFT 22

#ifndef __ASSEMBLY__

/** Integer representation of a PTE. */
typedef uint32_t pse_intpte_t;

static inline unsigned int pse_l1_table_offset(unsigned long va)
{
    return (va >> PSE_L1_PT_SHIFT) & (PSE_L1_PT_ENTRIES - 1);
}
static inline unsigned int pse_l2_table_offset(unsigned long va)
{
    return (va >> PSE_L2_PT_SHIFT) & (PSE_L2_PT_ENTRIES - 1);
}

#endif /* __ASSEMBLY__ */
#endif /* XTF_X86_PAGE_PSE_H */

/*
 * Local variables:
 * mode: C
 * c-file-style: "BSD"
 * c-basic-offset: 4
 * tab-width: 4
 * indent-tabs-mode: nil
 * End:
 */
