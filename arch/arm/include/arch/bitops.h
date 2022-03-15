/**
 * @file arch/arm/include/arch/bitops.h
 *
 * Low level bit operations.
 */
#ifndef XTF_ARM_BITOPS_H
#define XTF_ARM_BITOPS_H

#include <xtf/lib.h>

static inline bool test_bit(unsigned int bit, const void *addr)
{
    UNIMPLEMENTED();
    return false;
}

static inline bool test_and_set_bit(unsigned int bit, volatile void *addr)
{
    UNIMPLEMENTED();
    return false;
}

static inline bool test_and_change_bit(unsigned int bit, volatile void *addr)
{
    UNIMPLEMENTED();
    return false;
}

static inline bool test_and_clear_bit(unsigned int bit, volatile void *addr)
{
    UNIMPLEMENTED();
    return false;
}

#endif /* XTF_ARM_BITOPS_H */

/*
 * Local variables:
 * mode: C
 * c-file-style: "BSD"
 * c-basic-offset: 4
 * tab-width: 4
 * indent-tabs-mode: nil
 * End:
 */
