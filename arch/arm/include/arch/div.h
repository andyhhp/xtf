/**
 * @file arch/arm/include/arch/div.h
 */
#ifndef XTF_ARM_DIV_H
#define XTF_ARM_DIV_H

#include <xtf/lib.h>

/*
 * Divide a 64bit number by 32bit divisor without software support.
 *
 * The dividend is modified in place, and the modulus is returned.
 */
static inline uint32_t divmod64(uint64_t *dividend, uint32_t divisor)
{
#ifdef CONFIG_ARM_64
    uint32_t remainder = *dividend % divisor;
    *dividend = *dividend / divisor;
    return remainder;
#else
    UNIMPLEMENTED();
#endif
}

#endif /* XTF_ARM_DIV_H */

/*
 * Local variables:
 * mode: C
 * c-file-style: "BSD"
 * c-basic-offset: 4
 * tab-width: 4
 * indent-tabs-mode: nil
 * End:
 */
