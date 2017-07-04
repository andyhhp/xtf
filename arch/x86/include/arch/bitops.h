/**
 * @file arch/x86/include/arch/bitops.h
 *
 * Low level bit operations.
 */
#ifndef XTF_X86_BITOPS_H
#define XTF_X86_BITOPS_H

#include <xtf/lib.h>

static inline bool test_bit(unsigned int bit, const void *addr)
{
    bool old;

    asm volatile ("bt %[bit], %[ptr];"
                  ASM_FLAG_OUT(, "sbb %[old], %[old];")
                  : [old] ASM_FLAG_OUT("=@ccc", "=r") (old)
                  : [ptr] "m" (*(char *)addr),
                    [bit] "Ir" (bit)
                  : "memory");

    return old;
}

static inline bool test_and_set_bit(unsigned int bit, volatile void *addr)
{
    bool old;

    asm volatile ("lock; bts %[bit], %[ptr];"
                  ASM_FLAG_OUT(, "sbb %[old], %[old];")
                  : [old] ASM_FLAG_OUT("=@ccc", "=r") (old),
                    [ptr] "+m" (*(char *)addr)
                  : [bit] "Ir" (bit)
                  : "memory");

    return old;
}

static inline bool test_and_change_bit(unsigned int bit, volatile void *addr)
{
    bool old;

    asm volatile ("lock; btc %[bit], %[ptr];"
                  ASM_FLAG_OUT(, "sbb %[old], %[old];")
                  : [old] ASM_FLAG_OUT("=@ccc", "=r") (old),
                    [ptr] "+m" (*(char *)addr)
                  : [bit] "Ir" (bit)
                  : "memory");

    return old;
}

static inline bool test_and_clear_bit(unsigned int bit, volatile void *addr)
{
    bool old;

    asm volatile ("lock; btr %[bit], %[ptr];"
                  ASM_FLAG_OUT(, "sbb %[old], %[old];")
                  : [old] ASM_FLAG_OUT("=@ccc", "=r") (old),
                    [ptr] "+m" (*(char *)addr)
                  : [bit] "Ir" (bit)
                  : "memory");

    return old;
}

#endif /* XTF_X86_BITOPS_H */

/*
 * Local variables:
 * mode: C
 * c-file-style: "BSD"
 * c-basic-offset: 4
 * tab-width: 4
 * indent-tabs-mode: nil
 * End:
 */
