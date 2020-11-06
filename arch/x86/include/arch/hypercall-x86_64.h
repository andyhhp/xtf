#ifndef XTF_X86_64_HYPERCALL_H
#define XTF_X86_64_HYPERCALL_H

/*
 * Hypercall primatives for 64bit
 *
 * Inputs: %rdi, %rsi, %rdx, %r10, %r8, %r9 (arguments 1-6)
 */

#define _hypercall64_1(type, hcall, a1)                                 \
    ({                                                                  \
        long _res = (long)(hcall), _a1 = (long)(a1);                    \
        asm volatile (                                                  \
            "call hypercall_page + %c[offset]"                          \
            : "+a" (_res), "+D" (_a1)                                   \
            : [offset] "i" (hcall * 32)                                 \
            : "memory" );                                               \
        (type)_res;                                                     \
    })

#define _hypercall64_2(type, hcall, a1, a2)                             \
    ({                                                                  \
        long _res = (long)(hcall), _a1 = (long)(a1), _a2 = (long)(a2);  \
        asm volatile (                                                  \
            "call hypercall_page + %c[offset]"                          \
            : "+a" (_res), "+D" (_a1), "+S" (_a2)                       \
            : [offset] "i" (hcall * 32)                                 \
            : "memory" );                                               \
        (type)_res;                                                     \
    })

#define _hypercall64_3(type, hcall, a1, a2, a3)                         \
    ({                                                                  \
        long _res = (long)(hcall), _a1 = (long)(a1), _a2 = (long)(a2),  \
            _a3 = (long)(a3);                                           \
        asm volatile (                                                  \
            "call hypercall_page + %c[offset]"                          \
            : "+a" (_res), "+D" (_a1), "+S" (_a2), "+d" (_a3)           \
            : [offset] "i" (hcall * 32)                                 \
            : "memory" );                                               \
        (type)_res;                                                     \
    })

#define _hypercall64_4(type, hcall, a1, a2, a3, a4)                     \
    ({                                                                  \
        long _res = (long)(hcall), _a1 = (long)(a1), _a2 = (long)(a2),  \
            _a3 = (long)(a3);                                           \
        register long _a4 asm ("r10") = (long)(a4);                     \
        asm volatile (                                                  \
            "call hypercall_page + %c[offset]"                          \
            : "+a" (_res), "+D" (_a1), "+S" (_a2), "+d" (_a3),          \
              "+r" (_a4)                                                \
            : [offset] "i" (hcall * 32)                                 \
            : "memory" );                                               \
        (type)_res;                                                     \
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
