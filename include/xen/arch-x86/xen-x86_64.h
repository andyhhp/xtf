#ifndef XEN_PUBLIC_ARCH_X86_XEN_X86_64_H
#define XEN_PUBLIC_ARCH_X86_XEN_X86_64_H

/*
 * 64-bit segment selectors
 * These flat segments are in the Xen-private section of every GDT. Since these
 * are also present in the initial GDT, many OSes will be able to avoid
 * installing their own GDT.
 */

#define FLAT_RING3_CS32 0xe023  /* GDT index 260 */
#define FLAT_RING3_CS64 0xe033  /* GDT index 261 */
#define FLAT_RING3_DS32 0xe02b  /* GDT index 262 */
#define FLAT_RING3_DS64 0x0000  /* NULL selector */
#define FLAT_RING3_SS32 0xe02b  /* GDT index 262 */
#define FLAT_RING3_SS64 0xe02b  /* GDT index 262 */

#define MACH2PHYS_VIRT_START 0xFFFF800000000000UL

/* Guest exited in SYSCALL context? Return to guest with SYSRET? */
#define VGCF_in_syscall 0x100

#ifndef __ASSEMBLY__

/* Anonymous unions include all permissible names (e.g., al/ah/ax/eax/rax). */
#define __DECL_REG_LOHI(which) union { \
    uint64_t r ## which ## x; \
    uint32_t e ## which ## x; \
    uint16_t which ## x; \
    struct { \
        uint8_t which ## l; \
        uint8_t which ## h; \
    }; \
}
#define __DECL_REG_LO8(name) union { \
    uint64_t r ## name; \
    uint32_t e ## name; \
    uint16_t name; \
    uint8_t name ## l; \
}
#define __DECL_REG_LO16(name) union { \
    uint64_t r ## name; \
    uint32_t e ## name; \
    uint16_t name; \
}
#define __DECL_REG_HI(num) union { \
    uint64_t r ## num; \
    uint32_t r ## num ## d; \
    uint16_t r ## num ## w; \
    uint8_t r ## num ## b; \
}

struct xen_cpu_user_regs {
    __DECL_REG_HI(15);
    __DECL_REG_HI(14);
    __DECL_REG_HI(13);
    __DECL_REG_HI(12);
    __DECL_REG_LO8(bp);
    __DECL_REG_LOHI(b);
    __DECL_REG_HI(11);
    __DECL_REG_HI(10);
    __DECL_REG_HI(9);
    __DECL_REG_HI(8);
    __DECL_REG_LOHI(a);
    __DECL_REG_LOHI(c);
    __DECL_REG_LOHI(d);
    __DECL_REG_LO8(si);
    __DECL_REG_LO8(di);
    uint32_t error_code;    /* private */
    uint32_t entry_vector;  /* private */
    __DECL_REG_LO16(ip);
    uint16_t cs, _pad0[1];
    uint8_t  saved_upcall_mask;
    uint8_t  _pad1[3];
    __DECL_REG_LO16(flags); /* rflags.IF == !saved_upcall_mask */
    __DECL_REG_LO8(sp);
    uint16_t ss, _pad2[3];
    uint16_t es, _pad3[3];
    uint16_t ds, _pad4[3];
    uint16_t fs, _pad5[3]; /* Non-zero => takes precedence over fs_base.     */
    uint16_t gs, _pad6[3]; /* Non-zero => takes precedence over gs_base_usr. */
};

#undef __DECL_REG_LOHI
#undef __DECL_REG_LO8
#undef __DECL_REG_LO16
#undef __DECL_REG_HI

static inline unsigned long xen_pfn_to_cr3(unsigned long pfn)
{
    return pfn << 12;
}

static inline unsigned long xen_cr3_to_pfn(unsigned long cr3)
{
    return cr3 >> 12;
}

struct arch_vcpu_info {
    unsigned long cr2;
    unsigned long pad; /* sizeof(vcpu_info_t) == 64 */
};

typedef unsigned long xen_callback_t;

#define INIT_XEN_CALLBACK(_cs, _ip) (_ip)

#endif /* __ASSEMBLY__ */

#endif /* XEN_PUBLIC_ARCH_X86_XEN_X86_64_H */

/*
 * Local variables:
 * mode: C
 * c-file-style: "BSD"
 * c-basic-offset: 4
 * tab-width: 4
 * indent-tabs-mode: nil
 * End:
 */
