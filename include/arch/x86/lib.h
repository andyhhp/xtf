#ifndef XTF_X86_LIB_H
#define XTF_X86_LIB_H

#include <stdint.h>

static inline uint64_t rdmsr(uint32_t idx)
{
    uint32_t lo, hi;

    asm volatile("rdmsr": "=a" (lo), "=d" (hi): "c" (idx));

    return (((uint64_t)hi) << 32) | lo;
}

static inline void wrmsr(uint32_t idx, uint64_t val)
{
    asm volatile ("wrmsr":
                  : "c" (idx), "a" ((uint32_t)val),
                    "d" ((uint32_t)(val >> 32)));
}

static inline void cpuid(uint32_t leaf,
                         uint32_t *eax, uint32_t *ebx,
                         uint32_t *ecx, uint32_t *edx)
{
    asm volatile ("cpuid"
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

static inline unsigned int read_cs(void)
{
    unsigned int cs;

    asm volatile ("mov %%cs, %0" : "=r" (cs));

    return cs;
}

static inline unsigned int read_ds(void)
{
    unsigned int ds;

    asm volatile ("mov %%ds, %0" : "=r" (ds));

    return ds;
}

static inline unsigned int read_es(void)
{
    unsigned int es;

    asm volatile ("mov %%es, %0" : "=r" (es));

    return es;
}

static inline unsigned int read_fs(void)
{
    unsigned int fs;

    asm volatile ("mov %%fs, %0" : "=r" (fs));

    return fs;
}

static inline unsigned int read_gs(void)
{
    unsigned int gs;

    asm volatile ("mov %%gs, %0" : "=r" (gs));

    return gs;
}

static inline unsigned int read_ss(void)
{
    unsigned int ss;

    asm volatile ("mov %%ss, %0" : "=r" (ss));

    return ss;
}

static inline void write_ds(unsigned int ds)
{
    asm volatile ("mov %0, %%ds" :: "r" (ds));
}

static inline void write_es(unsigned int es)
{
    asm volatile ("mov %0, %%es" :: "r" (es));
}

static inline void write_fs(unsigned int fs)
{
    asm volatile ("mov %0, %%fs" :: "r" (fs));
}

static inline void write_gs(unsigned int gs)
{
    asm volatile ("mov %0, %%gs" :: "r" (gs));
}

static inline void write_ss(unsigned int ss)
{
    asm volatile ("mov %0, %%ss" :: "r" (ss));
}

static inline unsigned long read_dr6(void)
{
    unsigned long val;

    asm volatile ("mov %%dr6, %0" : "=r" (val));

    return val;
}

static inline unsigned long read_dr7(void)
{
    unsigned long val;

    asm volatile ("mov %%dr7, %0" : "=r" (val));

    return val;
}

static inline unsigned long read_cr3(void)
{
    unsigned long cr3;

    asm volatile ("mov %%cr3, %0" : "=r" (cr3));

    return cr3;
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
