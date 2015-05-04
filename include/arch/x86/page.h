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

#define PAGE_ORDER_4K           0
#define PAGE_ORDER_2M           9
#define PAGE_ORDER_1G           18

#define _PAGE_PRESENT           0x001
#define _PAGE_RW                0x002
#define _PAGE_USER              0x004
#define _PAGE_PSE               0x080

#define L1_PT_ENTRIES           512
#define L2_PT_ENTRIES           512
#define L3_PT_ENTRIES           512
#define L4_PT_ENTRIES           512

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
