#ifndef XTF_X86_PAGE_H
#define XTF_X86_PAGE_H

/*
 * Nomenclature inherited from Xen.
 */

#define PAGE_SHIFT              12

#ifdef __ASSEMBLY__
#define PAGE_SIZE               (1 << PAGE_SHIFT)
#else
#define PAGE_SIZE               (1L << PAGE_SHIFT)
#endif

#define PAGE_MASK               (~(PAGE_SIZE - 1))

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
