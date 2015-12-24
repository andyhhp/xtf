#ifndef XTF_X86_64_HYPERCALL_H
#define XTF_X86_64_HYPERCALL_H

/*
 * Hypercall primatives for 64bit
 *
 * Inputs: %rdi, %rsi, %rdx, %r10, %r8, %r9 (arguments 1-6)
 */

#define _hypercall64_1(type, hcall, a1)                                 \
    ({                                                                  \
        long __res, __ign1;                                             \
        asm volatile (                                                  \
            "call hypercall_page + %c[offset]"                          \
            : "=a" (__res), "=D" (__ign1)                               \
            : [offset] "i" (hcall * 32),                                \
              "1" ((long)(a1))                                          \
            : "memory" );                                               \
        (type)__res;                                                    \
    })

#define _hypercall64_2(type, hcall, a1, a2)                             \
    ({                                                                  \
        long __res, __ign1, __ign2;                                     \
        asm volatile (                                                  \
            "call hypercall_page + %c[offset]"                          \
            : "=a" (__res), "=D" (__ign1), "=S" (__ign2)                \
            : [offset] "i" (hcall * 32),                                \
              "1" ((long)(a1)), "2" ((long)(a2))                        \
            : "memory" );                                               \
        (type)__res;                                                    \
    })

#define _hypercall64_3(type, hcall, a1, a2, a3)                         \
    ({                                                                  \
        long __res, __ign1, __ign2, __ign3;                             \
        asm volatile (                                                  \
            "call hypercall_page + %c[offset]"                          \
            : "=a" (__res), "=D" (__ign1), "=S" (__ign2), "=d" (__ign3) \
            : [offset] "i" (hcall * 32),                                \
              "1" ((long)(a1)), "2" ((long)(a2)), "3" ((long)(a3))      \
            : "memory" );                                               \
        (type)__res;                                                    \
    })

#define _hypercall64_4(type, hcall, a1, a2, a3, a4)                     \
    ({                                                                  \
        long __res, __ign1, __ign2, __ign3, __ign4;                     \
        register long _a4 asm ("r10") = ((long)(a4));                   \
        asm volatile (                                                  \
            "call hypercall_page + %c[offset]"                          \
            : "=a" (__res), "=D" (__ign1), "=S" (__ign2), "=d" (__ign3),\
              "=&r" (__ign4)                                            \
            : [offset] "i" (hcall * 32),                                \
              "1" ((long)(a1)), "2" ((long)(a2)), "3" ((long)(a3)),     \
              "4" (_a4)                                                 \
            : "memory" );                                               \
        (type)__res;                                                    \
    })

#endif /* XTF_X86_64_HYPERCALL_H */

/*
 * Local variables:
 * mode: C
 * c-file-style: "BSD"
 * c-basic-offset: 4
 * tab-width: 4
 * indent-tabs-mode: nil
 * End:
 */
