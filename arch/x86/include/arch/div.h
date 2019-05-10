#ifndef XTF_X86_DIV_H
#define XTF_X86_DIV_H

#include <xtf/types.h>

/*
 * Divide a 64bit number by 32bit divisor without software support.
 *
 * The dividend is modified in place, and the modulus is returned.
 */
static inline uint32_t divmod64(uint64_t *dividend, uint32_t divisor)
{
    uint32_t mod;

#ifdef __x86_64__

    /*
     * On 64bit, issue a straight 'div' instruction.
     */

    mod = *dividend %  divisor;
          *dividend /= divisor;
#else
    {
        /*
         * On 32bit, this is harder.
         *
         * In x86, 'divl' can take a 64bit dividend, but the resulting
         * quotient must fit in %eax or a #DE will occur.
         *
         * To avoid this, we split the division in two.  The remainder from
         * the higher divide can safely be used in the upper 32bits of the
         * lower divide, as it will not cause an overflow.
         */
        uint32_t high = *dividend >> 32, low = *dividend, umod = 0;

        if ( high )
        {
            umod = high %  divisor;
                   high /= divisor;
        }

        asm ("divl %[divisor]"
             : "+a" (low), "=d" (mod)
             : [divisor] "rm" (divisor), "d" (umod));

        *dividend = (((uint64_t)high) << 32) | low;
    }
#endif

    return mod;
}

#endif /* XTF_X86_DIV_H */

/*
 * Local variables:
 * mode: C
 * c-file-style: "BSD"
 * c-basic-offset: 4
 * tab-width: 4
 * indent-tabs-mode: nil
 * End:
 */
