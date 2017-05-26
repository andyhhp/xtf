/**
 * @file arch/x86/include/arch/page-pse.h
 *
 * Definitions and helpers for PSE pagetable handling.
 */
#ifndef XTF_X86_PAGE_PSE_H
#define XTF_X86_PAGE_PSE_H

/** PSE pagetable entries are 32 bits wide. */
#define PSE_PTE_SIZE 4
/** PSE pagetable entries are 32 bits wide. */
#define PSE_PTE_ORDER 2

/** PAE pagetables encode 10 bits of index. */
#define PSE_PT_ORDER 10

/** @{ */
/** All PSE pagetables contain 1024 entries. */
#define PSE_L1_PT_ENTRIES (PAGE_SIZE / PSE_PTE_SIZE)
#define PSE_L2_PT_ENTRIES (PAGE_SIZE / PSE_PTE_SIZE)
/** @} */

#define PSE_L1_PT_SHIFT 12
#define PSE_L2_PT_SHIFT 22

#ifndef __ASSEMBLY__

/** Integer representation of a PTE. */
typedef uint32_t pse_intpte_t;
#define PSE_PRIpte "08"PRIx32

static inline unsigned int pse_l1_table_offset(unsigned long linear)
{
    return (linear >> PSE_L1_PT_SHIFT) & (PSE_L1_PT_ENTRIES - 1);
}
static inline unsigned int pse_l2_table_offset(unsigned long linear)
{
    return (linear >> PSE_L2_PT_SHIFT) & (PSE_L2_PT_ENTRIES - 1);
}

static inline uint32_t fold_pse36(uint64_t val)
{
    return (val & ~(0x1ffULL << 13)) | ((val & (0x1ffULL << 32)) >> (32 - 13));
}

static inline uint64_t unfold_pse36(uint32_t val)
{
    return (val & ~(0x1ffULL << 13)) | ((val & (0x1ffULL << 13)) << (32 - 13));
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
