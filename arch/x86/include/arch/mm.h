#ifndef XTF_X86_MM_H
#define XTF_X86_MM_H

#include <xtf/types.h>

#include <arch/page.h>

#include <xen/xen.h>

/*
 * Terminology (inherited from Xen):
 *
 *   GFN - Guest Frame Number
 *           What a guest writes into its pagetables.
 *   MFN - Machine Frame Number
 *           What Xen writes into its pagetables.
 *   PFN - Pseudophysical Frame Number
 *           A linear idea of a guests physical address space.
 *
 * For HVM, PFN == GFN, and MFN is strictly irrelevent.
 * For PV,  MFN == GFN != PFN.
 *
 * XTF memory layout.
 *
 * Wherever possible, identity layout for simplicity.
 *
 * PV guests: VIRT_OFFSET is 0 which causes all linked virtual addresses to be
 * contiguous in the pagetables created by the domain builder.  Therefore,
 * virt == pfn << PAGE_SHIFT for any pfn constructed by the domain builder.
 *
 * HVM guests: All memory from 0 to 4GB is identity mapped.
 */

static inline void *pfn_to_virt(unsigned long pfn)
{
    return (void *)(pfn << PAGE_SHIFT);
}

static inline unsigned long virt_to_pfn(const void *va)
{
    return _u(va) >> PAGE_SHIFT;
}

#if defined(CONFIG_PV)

#define m2p ((unsigned long *)MACH2PHYS_VIRT_START)
extern start_info_t *start_info;

static inline void *mfn_to_virt(unsigned long mfn)
{
    return pfn_to_virt(m2p[mfn]);
}

static inline void *maddr_to_virt(uint64_t maddr)
{
    return mfn_to_virt(maddr >> PAGE_SHIFT) + (maddr & ~PAGE_MASK);
}

static inline unsigned long pfn_to_mfn(unsigned long pfn)
{
    unsigned long *p2m = _p(start_info->mfn_list);

    return p2m[pfn];
}

static inline unsigned long virt_to_mfn(const void *va)
{
    return pfn_to_mfn(virt_to_pfn(va));
}

#undef m2p

#endif /* CONFIG_PV */

static inline void *gfn_to_virt(unsigned long gfn)
{
#if defined(CONFIG_PV)
    return mfn_to_virt(gfn);
#else
    return pfn_to_virt(gfn);
#endif
}

static inline unsigned long virt_to_gfn(const void *va)
{
#if defined(CONFIG_PV)
    return virt_to_mfn(va);
#else
    return virt_to_pfn(va);
#endif
}

#endif /* XTF_X86_MM_H */

/*
 * Local variables:
 * mode: C
 * c-file-style: "BSD"
 * c-basic-offset: 4
 * tab-width: 4
 * indent-tabs-mode: nil
 * End:
 */
