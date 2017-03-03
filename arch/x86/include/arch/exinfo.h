/**
 * @file arch/x86/include/arch/exinfo.h
 *
 * An encapsulation of an x86 exception with error code
 */
#ifndef XTF_X86_EXINFO_H
#define XTF_X86_EXINFO_H

#include <arch/processor.h>

/**
 * Packed exception and error code information
 *
 * - Bottom 16 bits are error code
 * - Next 8 bits are the entry vector
 * - Top bit it set to disambiguate @#DE from no exception
 */
typedef unsigned int exinfo_t;

#define EXINFO_EXPECTED (1u << 31)

#define EXINFO(vec, ec) (EXINFO_EXPECTED | ((vec & 0xff) << 16) | (ec & 0xffff))

#define EXINFO_SYM(exc, ec) EXINFO(X86_EXC_ ## exc, ec)

static inline unsigned int exinfo_vec(exinfo_t info)
{
    return (info >> 16) & 0xff;
}

static inline unsigned int exinfo_ec(exinfo_t info)
{
    return info & 0xffff;
}

#endif /* XTF_X86_EXINFO_H */

/*
 * Local variables:
 * mode: C
 * c-file-style: "BSD"
 * c-basic-offset: 4
 * tab-width: 4
 * indent-tabs-mode: nil
 * End:
 */
