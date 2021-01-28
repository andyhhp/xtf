#ifndef XTF_X86_32_HYPERCALL_H
#define XTF_X86_32_HYPERCALL_H

/*
 * Hypercall primatives for 32bit
 *
 * Inputs: %ebx, %ecx, %edx, %esi, %edi, %ebp (arguments 1-6)
 */

#define _hypercall32_0(type, hcall)                                     \
    ({                                                                  \
        long res;                                                       \
        asm volatile (                                                  \
            "call hypercall_page + %c[offset]"                          \
            : "=a" (res)                                                \
            : [offset] "i" (hcall * 32)                                 \
            : "memory" );                                               \
        (type)res;                                                      \
    })

#define _hypercall32_1(type, hcall, a1)                                 \
    ({                                                                  \
        long res, _a1 = (long)(a1);                                     \
        asm volatile (                                                  \
            "call hypercall_page + %c[offset]"                          \
            : "=a" (res), "+b" (_a1)                                    \
            : [offset] "i" (hcall * 32)                                 \
            : "memory" );                                               \
        (type)res;                                                      \
    })

#define _hypercall32_2(type, hcall, a1, a2)                             \
    ({                                                                  \
        long res, _a1 = (long)(a1), _a2 = (long)(a2);                   \
        asm volatile (                                                  \
            "call hypercall_page + %c[offset]"                          \
            : "=a" (res), "+b" (_a1), "+c" (_a2)                        \
            : [offset] "i" (hcall * 32)                                 \
            : "memory" );                                               \
        (type)res;                                                      \
    })

#define _hypercall32_3(type, hcall, a1, a2, a3)                         \
    ({                                                                  \
        long res, _a1 = (long)(a1), _a2 = (long)(a2), _a3 = (long)(a3); \
        asm volatile (                                                  \
            "call hypercall_page + %c[offset]"                          \
            : "=a" (res), "+b" (_a1), "+c" (_a2), "+d" (_a3)            \
            : [offset] "i" (hcall * 32)                                 \
            : "memory" );                                               \
        (type)res;                                                      \
    })

#define _hypercall32_4(type, hcall, a1, a2, a3, a4)                     \
    ({                                                                  \
        long res, _a1 = (long)(a1), _a2 = (long)(a2), _a3 = (long)(a3), \
            _a4 = (long)(a4);                                           \
        asm volatile (                                                  \
            "call hypercall_page + %c[offset]"                          \
            : "=a" (res), "+b" (_a1), "+c" (_a2), "+d" (_a3),           \
              "+S" (_a4)                                                \
            : [offset] "i" (hcall * 32)                                 \
            : "memory" );                                               \
        (type)res;                                                      \
    })

#define _hypercall32_5(type, hcall, a1, a2, a3, a4, a5)                 \
    ({                                                                  \
        long res, _a1 = (long)(a1), _a2 = (long)(a2), _a3 = (long)(a3), \
            _a4 = (long)(a4), _a5 = (long)(a5);                         \
        asm volatile (                                                  \
            "call hypercall_page + %c[offset]"                          \
            : "=a" (res), "+b" (_a1), "+c" (_a2), "+d" (_a3),           \
              "+S" (_a4), "+D" (_a5)                                    \
            : [offset] "i" (hcall * 32)                                 \
            : "memory" );                                               \
        (type)res;                                                      \
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
