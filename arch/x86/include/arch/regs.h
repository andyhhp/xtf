#ifndef XTF_X86_REGS_H
#define XTF_X86_REGS_H

#include <xtf/types.h>

#if defined(__i386__)

#define DECL_REG(n)                             \
    union { uint32_t e ## n; unsigned long n; }
#define _DECL_REG(n)                            \
    union { uint32_t _e ## n; unsigned long _ ## n; }

struct cpu_regs {
    DECL_REG(bp);
    DECL_REG(bx);
    DECL_REG(ax);
    DECL_REG(cx);
    DECL_REG(dx);
    DECL_REG(si);
    DECL_REG(di);

    uint32_t entry_vector;
    uint32_t error_code;

/* Hardware exception frame. */
    DECL_REG(ip);
    uint16_t cs, _pad1[1];
    DECL_REG(flags);
    _DECL_REG(sp);          /* Won't be valid if stack */
    uint16_t _ss, _pad0[1]; /* switch didn't occur.    */
/* Top of stack. */
};

#elif defined(__x86_64__)

#define DECL_REG(n)                             \
    union { uint64_t r ## n; uint32_t e ## n; unsigned long n; }
#define _DECL_REG(n)                                                \
    union { uint64_t _r ## n; uint32_t _e ## n; unsigned long _ ## n; }

struct cpu_regs {
    uint64_t r15;
    uint64_t r14;
    uint64_t r13;
    uint64_t r12;
    DECL_REG(bp);
    DECL_REG(bx);
    uint64_t r11;
    uint64_t r10;
    uint64_t r9;
    uint64_t r8;
    DECL_REG(ax);
    DECL_REG(cx);
    DECL_REG(dx);
    DECL_REG(si);
    DECL_REG(di);

    uint32_t error_code;
    uint32_t entry_vector;

/* Hardware exception frame. */
    DECL_REG(ip);
    uint16_t cs, _pad1[3];
    DECL_REG(flags);
    _DECL_REG(sp);
    uint16_t _ss, _pad0[3];
/* Top of stack. */
};

#endif /* __i386__ / __x86_64__ */

#endif /* XTF_X86_REGS_H */

/*
 * Local variables:
 * mode: C
 * c-file-style: "BSD"
 * c-basic-offset: 4
 * tab-width: 4
 * indent-tabs-mode: nil
 * End:
 */
