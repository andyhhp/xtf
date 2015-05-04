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
