#ifndef XTF_COMPILER_H
#define XTF_COMPILER_H

#define __aligned(x)          __attribute__((aligned(x)))
#define __packed              __attribute__((packed))

#define __noreturn            __attribute__((noreturn))
#define unreachable()         __builtin_unreachable()

#define __printf(f, v)        __attribute__((format(__printf__, f, v)))

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
