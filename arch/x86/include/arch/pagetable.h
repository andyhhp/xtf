#ifndef XTF_X86_PAGETABLE_H
#define XTF_X86_PAGETABLE_H

#include <arch/mm.h>

#if CONFIG_PAGING_LEVELS > 0

static inline paddr_t pte_to_paddr(intpte_t pte)
{
    return pte & PADDR_MASK & PAGE_MASK;
}

static inline intpte_t pte_from_paddr(paddr_t paddr, uint64_t flags)
{
    return ((paddr &  (PADDR_MASK & PAGE_MASK)) |
            (flags & ~(PADDR_MASK & PAGE_MASK)));
}

static inline intpte_t pte_from_gfn(unsigned long gfn, uint64_t flags)
{
    return pte_from_paddr((paddr_t)gfn << PAGE_SHIFT, flags);
}

static inline intpte_t pte_from_virt(const void *va, uint64_t flags)
{
    return pte_from_paddr((paddr_t)virt_to_gfn(va) << PAGE_SHIFT, flags);
}

#else /* CONFIG_PAGING_LEVELS > 0 */

/* Enough compatibility to compile in unpaged environments. */
paddr_t pte_to_paddr(intpte_t pte);
intpte_t pte_from_paddr(paddr_t paddr, uint64_t flags);
intpte_t pte_from_gfn(unsigned long gfn, uint64_t flags);
intpte_t pte_from_virt(const void *va, uint64_t flags);

#endif

#endif /* XTF_X86_PAGETABLE_H */

/*
 * Local variables:
 * mode: C
 * c-file-style: "BSD"
 * c-basic-offset: 4
 * tab-width: 4
 * indent-tabs-mode: nil
 * End:
 */
