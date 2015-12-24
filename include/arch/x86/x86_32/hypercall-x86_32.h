#ifndef XTF_X86_32_HYPERCALL_H
#define XTF_X86_32_HYPERCALL_H

/*
 * Hypercall primatives for 32bit
 *
 * Inputs: %ebx, %ecx, %edx, %esi, %edi, %ebp (arguments 1-6)
 */

#define _hypercall32_1(type, hcall, a1)                                 \
    ({                                                                  \
        long __res, __ign1;                                             \
        asm volatile (                                                  \
            "call hypercall_page + %c[offset]"                          \
            : "=a" (__res), "=b" (__ign1)                               \
            : [offset] "i" (hcall * 32),                                \
              "1" ((long)(a1))                                          \
            : "memory" );                                               \
        (type)__res;                                                    \
    })

#define _hypercall32_2(type, hcall, a1, a2)                             \
    ({                                                                  \
        long __res, __ign1, __ign2;                                     \
        asm volatile (                                                  \
            "call hypercall_page + %c[offset]"                          \
            : "=a" (__res), "=b" (__ign1), "=c" (__ign2)                \
            : [offset] "i" (hcall * 32),                                \
              "1" ((long)(a1)), "2" ((long)(a2))                        \
            : "memory" );                                               \
        (type)__res;                                                    \
    })

#define _hypercall32_3(type, hcall, a1, a2, a3)                         \
    ({                                                                  \
        long __res, __ign1, __ign2, __ign3;                             \
        asm volatile (                                                  \
            "call hypercall_page + %c[offset]"                          \
            : "=a" (__res), "=b" (__ign1), "=c" (__ign2), "=d" (__ign3) \
            : [offset] "i" (hcall * 32),                                \
              "1" ((long)(a1)), "2" ((long)(a2)), "3" ((long)(a3))      \
            : "memory" );                                               \
        (type)__res;                                                    \
    })

#define _hypercall32_4(type, hcall, a1, a2, a3, a4)                     \
    ({                                                                  \
        long __res, __ign1, __ign2, __ign3, __ign4;                     \
        asm volatile (                                                  \
            "call hypercall_page + %c[offset]"                          \
            : "=a" (__res), "=b" (__ign1), "=c" (__ign2), "=d" (__ign3),\
              "=S" (__ign4)                                             \
            : [offset] "i" (hcall * 32),                                \
              "1" ((long)(a1)), "2" ((long)(a2)), "3" ((long)(a3)),     \
              "4" ((long)(a4))                                          \
            : "memory" );                                               \
        (type)__res;                                                    \
    })

#endif /* XTF_X86_32_HYPERCALL_H */

/*
 * Local variables:
 * mode: C
 * c-file-style: "BSD"
 * c-basic-offset: 4
 * tab-width: 4
 * indent-tabs-mode: nil
 * End:
 */
