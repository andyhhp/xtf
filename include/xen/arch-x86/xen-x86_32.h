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

/* Anonymous unions include all permissible names (e.g., al/ah/ax/eax). */
#define __DECL_REG_LO8(which) union { \
    uint32_t e ## which ## x; \
    uint16_t which ## x; \
    struct { \
        uint8_t which ## l; \
        uint8_t which ## h; \
    }; \
}
#define __DECL_REG_LO16(name) union { \
    uint32_t e ## name, _e ## name; \
    uint16_t name; \
}

struct xen_cpu_user_regs {
    __DECL_REG_LO8(b);
    __DECL_REG_LO8(c);
    __DECL_REG_LO8(d);
    __DECL_REG_LO16(si);
    __DECL_REG_LO16(di);
    __DECL_REG_LO16(bp);
    __DECL_REG_LO8(a);
    uint16_t error_code;    /* private */
    uint16_t entry_vector;  /* private */
    __DECL_REG_LO16(ip);
    uint16_t cs;
    uint8_t  saved_upcall_mask;
    uint8_t  _pad0;
    __DECL_REG_LO16(flags); /* eflags.IF == !saved_upcall_mask */
    __DECL_REG_LO16(sp);
    uint16_t ss, _pad1;
    uint16_t es, _pad2;
    uint16_t ds, _pad3;
    uint16_t fs, _pad4;
    uint16_t gs, _pad5;
};

#undef __DECL_REG_LO8
#undef __DECL_REG_LO16

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

struct arch_vcpu_info {
    unsigned long cr2;
    unsigned long pad[5]; /* sizeof(vcpu_info_t) == 64 */
};

struct xen_callback {
    unsigned long cs;
    unsigned long eip;
};
typedef struct xen_callback xen_callback_t;

#define INIT_XEN_CALLBACK(_cs, _ip) \
    ((xen_callback_t){ .cs = _cs, .eip = _ip })

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
