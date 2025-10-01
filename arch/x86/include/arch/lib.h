#ifndef XTF_X86_LIB_H
#define XTF_X86_LIB_H

#include <xtf/types.h>
#include <xtf/extable.h>
#include <xen/arch-x86/xen.h>
#include <arch/desc.h>
#include <arch/msr.h>

static inline void cpuid(uint32_t leaf,
                         uint32_t *eax, uint32_t *ebx,
                         uint32_t *ecx, uint32_t *edx)
{
    asm volatile ("cpuid"
                  : "=a" (*eax), "=b" (*ebx), "=c" (*ecx), "=d" (*edx)
                  : "0" (leaf));
}

static inline uint32_t cpuid_eax(uint32_t leaf)
{
    uint32_t eax, tmp;

    cpuid(leaf, &eax, &tmp, &tmp, &tmp);

    return eax;
}

static inline uint32_t cpuid_ebx(uint32_t leaf)
{
    uint32_t ebx, tmp;

    cpuid(leaf, &tmp, &ebx, &tmp, &tmp);

    return ebx;
}

static inline uint32_t cpuid_ecx(uint32_t leaf)
{
    uint32_t ecx, tmp;

    cpuid(leaf, &tmp, &tmp, &ecx, &tmp);

    return ecx;
}

static inline uint32_t cpuid_edx(uint32_t leaf)
{
    uint32_t edx, tmp;

    cpuid(leaf, &tmp, &tmp, &tmp, &edx);

    return edx;
}

static inline void pv_cpuid(uint32_t leaf,
                            uint32_t *eax, uint32_t *ebx,
                            uint32_t *ecx, uint32_t *edx)
{
    asm volatile (_ASM_XEN_FEP "cpuid"
                  : "=a" (*eax), "=b" (*ebx), "=c" (*ecx), "=d" (*edx)
                  : "0" (leaf));
}

static inline void cpuid_count(uint32_t leaf, uint32_t subleaf,
                               uint32_t *eax, uint32_t *ebx,
                               uint32_t *ecx, uint32_t *edx)
{
    asm volatile ("cpuid"
                  : "=a" (*eax), "=b" (*ebx), "=c" (*ecx), "=d" (*edx)
                  : "0" (leaf), "2" (subleaf));
}

static inline void pv_cpuid_count(uint32_t leaf, uint32_t subleaf,
                                  uint32_t *eax, uint32_t *ebx,
                                  uint32_t *ecx, uint32_t *edx)
{
    asm volatile (_ASM_XEN_FEP "cpuid"
                  : "=a" (*eax), "=b" (*ebx), "=c" (*ecx), "=d" (*edx)
                  : "0" (leaf), "2" (subleaf));
}

static inline uint8_t inb(uint16_t port)
{
    uint8_t val;

    asm volatile("inb %w1, %b0": "=a" (val): "Nd" (port));

    return val;
}

static inline uint16_t inw(uint16_t port)
{
    uint16_t val;

    asm volatile("inw %w1, %w0": "=a" (val): "Nd" (port));

    return val;
}

static inline uint32_t inl(uint16_t port)
{
    uint32_t val;

    asm volatile("inl %w1, %k0": "=a" (val): "Nd" (port));

    return val;
}

static inline void outb(uint8_t val, uint16_t port)
{
    asm volatile("outb %b0, %w1": : "a" (val), "Nd" (port));
}

static inline void outw(uint16_t val, uint16_t port)
{
    asm volatile("outw %w0, %w1": : "a" (val), "Nd" (port));
}

static inline void outl(uint32_t val, uint16_t port)
{
    asm volatile("outl %k0, %w1": : "a" (val), "Nd" (port));
}

static inline void rep_outsb(const char *buf, size_t len, uint16_t port)
{
    asm volatile ("rep outsb"
                  : "+S" (buf), "+c" (len)
                  : "d" (port) );
}

static inline unsigned int read_cs(void)
{
    unsigned int cs;

    asm volatile ("mov %%cs, %0" : "=rm" (cs));

    return cs;
}

static inline unsigned int read_ds(void)
{
    unsigned int ds;

    asm volatile ("mov %%ds, %0" : "=rm" (ds));

    return ds;
}

static inline unsigned int read_es(void)
{
    unsigned int es;

    asm volatile ("mov %%es, %0" : "=rm" (es));

    return es;
}

static inline unsigned int read_fs(void)
{
    unsigned int fs;

    asm volatile ("mov %%fs, %0" : "=rm" (fs));

    return fs;
}

static inline unsigned int read_gs(void)
{
    unsigned int gs;

    asm volatile ("mov %%gs, %0" : "=rm" (gs));

    return gs;
}

static inline unsigned int read_ss(void)
{
    unsigned int ss;

    asm volatile ("mov %%ss, %0" : "=rm" (ss));

    return ss;
}

static inline void write_cs(unsigned long cs)
{
    asm volatile ("push %0;"
                  "push $1f;"
                  __ASM_SEL(lretl, lretq) "; 1:"
                  :: "rme" (cs));
}

static inline void write_ds(unsigned int ds)
{
    asm volatile ("mov %0, %%ds" :: "rm" (ds));
}

static inline void write_es(unsigned int es)
{
    asm volatile ("mov %0, %%es" :: "rm" (es));
}

static inline void write_fs(unsigned int fs)
{
    asm volatile ("mov %0, %%fs" :: "rm" (fs));
}

static inline void write_gs(unsigned int gs)
{
    asm volatile ("mov %0, %%gs" :: "rm" (gs));
}

static inline void write_ss(unsigned int ss)
{
    asm volatile ("mov %0, %%ss" :: "rm" (ss));
}

static inline unsigned long read_flags(void)
{
    unsigned long flags;

    asm volatile ("pushf; pop %0" : "=rm" (flags));

    return flags;
}

static inline void write_flags(unsigned long flags)
{
    asm volatile ("push %0; popf" :: "rme" (flags));
}

static inline unsigned long read_cr0(void)
{
    unsigned long cr0;

    asm volatile ("mov %%cr0, %0" : "=r" (cr0));

    return cr0;
}

static inline unsigned long read_cr2(void)
{
    unsigned long cr2;

    asm volatile ("mov %%cr2, %0" : "=r" (cr2));

    return cr2;
}

static inline unsigned long read_cr3(void)
{
    unsigned long cr3;

    asm volatile ("mov %%cr3, %0" : "=r" (cr3));

    return cr3;
}

static inline unsigned long read_cr4(void)
{
    unsigned long cr4;

    asm volatile ("mov %%cr4, %0" : "=r" (cr4));

    return cr4;
}

static inline unsigned long read_cr8(void)
{
    unsigned long cr8;

    asm volatile ("mov %%cr8, %0" : "=r" (cr8));

    return cr8;
}

static inline void write_cr0(unsigned long cr0)
{
    asm volatile ("mov %0, %%cr0" :: "r" (cr0));
}

static inline void write_cr2(unsigned long cr2)
{
    asm volatile ("mov %0, %%cr2" :: "r" (cr2));
}

static inline void write_cr3(unsigned long cr3)
{
    asm volatile ("mov %0, %%cr3" :: "r" (cr3));
}

static inline void write_cr4(unsigned long cr4)
{
    asm volatile ("mov %0, %%cr4" :: "r" (cr4));
}

static inline bool write_cr4_safe(unsigned long cr4)
{
    exinfo_t fault = 0;

    asm volatile ("1: mov %[cr4], %%cr4; 2:"
                  _ASM_EXTABLE_HANDLER(1b, 2b, %P[rec])
                  : "+D" (fault)
                  : [cr4] "r" (cr4),
                    [rec] "p" (ex_record_fault_edi));

    return fault;
}

static inline void write_cr8(unsigned long cr8)
{
    asm volatile ("mov %0, %%cr8" :: "r" (cr8));
}

static inline uint32_t read_mxcsr(void)
{
    uint32_t mxcsr;

    asm volatile ("stmxcsr %0" : "=m" (mxcsr));

    return mxcsr;
}

static inline void write_mxcsr(uint32_t mxcsr)
{
    asm volatile ("ldmxcsr %0" :: "m" (mxcsr));
}

static inline void invlpg(const void *va)
{
    asm volatile ("invlpg (%0)" :: "r" (va));
}

static inline void lgdt(const desc_ptr *gdtr)
{
    asm volatile ("lgdt %0" :: "m" (*gdtr));
}

static inline void lidt(const desc_ptr *idtr)
{
    asm volatile ("lidt %0" :: "m" (*idtr));
}

static inline void lldt(unsigned int sel)
{
    asm volatile ("lldt %w0" :: "rm" (sel));
}

static inline void ltr(unsigned int sel)
{
    asm volatile ("ltr %w0" :: "rm" (sel));
}

static inline void sgdt(desc_ptr *gdtr)
{
    asm volatile ("sgdt %0" : "=m" (*gdtr));
}

static inline void sidt(desc_ptr *idtr)
{
    asm volatile ("sidt %0" : "=m" (*idtr));
}

static inline unsigned int sldt(void)
{
    unsigned int sel;

    asm volatile ("sldt %0" : "=r" (sel));

    return sel;
}

static inline unsigned int str(void)
{
    unsigned int sel;

    asm volatile ("str %0" : "=r" (sel));

    return sel;
}

static inline uint64_t xgetbv(uint32_t index)
{
    uint32_t feat_lo, feat_hi;

    asm volatile ("xgetbv" : "=a" (feat_lo), "=d" (feat_hi)
                           :  "c" (index) );

    return feat_lo | ((uint64_t)feat_hi << 32);
}

static inline void xsetbv(uint32_t index, uint64_t value)
{
    asm volatile ("xsetbv" :: "a" ((uint32_t)value),
                              "d" ((uint32_t)(value >> 32)),
                              "c" (index) );
}

static inline uint64_t read_xcr0(void)
{
    return xgetbv(0);
}

static inline void write_xcr0(uint64_t xcr0)
{
    xsetbv(0, xcr0);
}

static inline void clflush(const void *ptr)
{
    asm volatile ("clflush %0" :: "m" (*(const char *)ptr));
}

static inline void flush_tlb(void)
{
    write_cr3(read_cr3());
}

#endif /* XTF_X86_LIB_H */

/*
 * Local variables:
 * mode: C
 * c-file-style: "BSD"
 * c-basic-offset: 4
 * tab-width: 4
 * indent-tabs-mode: nil
 * End:
 */
