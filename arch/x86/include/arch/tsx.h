/**
 * @file arch/x86/include/arch/tsx.h
 *
 * %x86 TSX intrinsics, compatible with the GCC API.
 *
 * To use:
 *
 * ~~~~~{.c}
 *
 *    if ( (xstatus = _xbegin()) == _XBEGIN_STARTED )
 *    {
 *        // Inside hardware transaction
 *        _xend();
 *    }
 *    else
 *        // Abort occured
 *
 * ~~~~~
 */
#ifndef XTF_X86_TSX_H
#define XTF_X86_TSX_H

#include <xtf/compiler.h>

#define _XBEGIN_STARTED  (~0u)
#define _XABORT_EXPLICIT (1u << 0)
#define _XABORT_RETRY    (1u << 1)
#define _XABORT_CONFLICT (1u << 2)
#define _XABORT_CAPACITY (1u << 3)
#define _XABORT_DEBUG    (1u << 4)
#define _XABORT_NESTED   (1u << 5)
#define _XABORT_CODE(x)  (((x) >> 24) & 0xff)

static inline unsigned int _xbegin(void)
{
    unsigned int ret = _XBEGIN_STARTED;

    asm volatile (".byte 0xc7, 0xf8, 0, 0, 0, 0" /* xbegin 1f; 1: */
                  : "+a" (ret) :: "memory");

    return ret;
}

static inline int _xtest(void)
{
    int rc;

    asm volatile (".byte 0x0f, 0x01, 0xd6" /* xtest */
                  ASM_FLAG_OUT(, "; setnz %[rc]")
                  : ASM_FLAG_OUT("=@ccnz", [rc] "=rm") (rc));

    return rc;
}

/*
 * N.B. Should be static inline, but clang can't cope with 'code' being
 * propagated through a function parameter.
 */
#define _xabort(code)                                           \
    do {                                                        \
        asm volatile (".byte 0xc6, 0xf8, %c0" /* xabort %0 */   \
                      :: "N" (code) : "memory");                \
        unreachable();                                          \
    } while ( 0 )

static inline void _xend(void)
{
    asm volatile (".byte 0x0f, 0x01, 0xd6" /* xend */
                  ::: "memory");
}

#endif /* XTF_X86_TSX_H */

/*
 * Local variables:
 * mode: C
 * c-file-style: "BSD"
 * c-basic-offset: 4
 * tab-width: 4
 * indent-tabs-mode: nil
 * End:
 */
