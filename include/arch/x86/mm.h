#ifndef XTF_X86_MM_H
#define XTF_X86_MM_H

#include <stdint.h>

#include <arch/x86/config.h>
#include <arch/x86/page.h>

#include <xen/xen.h>

/*
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

#if defined(CONFIG_ENV_pv)

#define m2p ((unsigned long *)MACH2PHYS_VIRT_START)

static inline void *mfn_to_virt(unsigned long mfn)
{
    return pfn_to_virt(m2p[mfn]);
}

#undef m2p

#endif /* CONFIG_ENV_pv */

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
