#ifndef XEN_PUBLIC_ARCH_X86_XEN_X86_32_H
#define XEN_PUBLIC_ARCH_X86_XEN_X86_32_H

/*
 * These flat segments are in the Xen-private section of every GDT. Since these
 * are also present in the initial GDT, many OSes will be able to avoid
 * installing their own GDT.
 */
#define FLAT_RING1_CS 0xe019    /* GDT index 259 */
#define FLAT_RING1_DS 0xe021    /* GDT index 260 */
#define FLAT_RING1_SS 0xe021    /* GDT index 260 */
#define FLAT_RING3_CS 0xe02b    /* GDT index 261 */
#define FLAT_RING3_DS 0xe033    /* GDT index 262 */
#define FLAT_RING3_SS 0xe033    /* GDT index 262 */

#define MACH2PHYS_VIRT_START 0xF5800000UL

#define __HYPERVISOR_VIRT_START_PAE 0xF5800000UL

#ifndef __ASSEMBLY__

/*
 * Page-directory addresses above 4GB do not fit into architectural %cr3.
 * When accessing %cr3, or equivalent field in vcpu_guest_context, guests
 * must use the following accessor macros to pack/unpack valid MFNs.
 */
static inline unsigned int xen_pfn_to_cr3(unsigned int pfn)
{
    return pfn << 12 | pfn >> 20;
}

static inline unsigned int xen_cr3_to_pfn(unsigned int cr3)
{
    return cr3 >> 12 | cr3 << 20;
}

struct xen_callback {
    unsigned long cs;
    unsigned long eip;
};
typedef struct xen_callback xen_callback_t;

#endif /* __ASSEMBLY__ */

#endif /* XEN_PUBLIC_ARCH_X86_XEN_X86_32_H */

/*
 * Local variables:
 * mode: C
 * c-file-style: "BSD"
 * c-basic-offset: 4
 * tab-width: 4
 * indent-tabs-mode: nil
 * End:
 */
