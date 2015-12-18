#ifndef XTF_COMPILER_H
#define XTF_COMPILER_H

#define __aligned(x)          __attribute__((__aligned__(x)))
#define __packed              __attribute__((__packed__))

#define __noinline            __attribute__((__noinline__))

#ifndef __always_inline /* Avoid conflicting with cdefs.h */
#define __always_inline       __attribute__((__always_inline__))
#endif

#define __noreturn            __attribute__((__noreturn__))
#define unreachable()         __builtin_unreachable()

#define __printf(f, v)        __attribute__((__format__(__printf__, f, v)))

#define barrier()             __asm__ __volatile__ ("" ::: "memory")

#endif /* XTF_COMPILER_H */

/*
 * Local variables:
 * mode: C
 * c-file-style: "BSD"
 * c-basic-offset: 4
 * tab-width: 4
 * indent-tabs-mode: nil
 * End:
 */
