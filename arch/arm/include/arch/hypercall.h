/**
 * @file arch/arm/include/arch/hypercall.h
 *
 * Hypercall primitives for arm.
 */
#ifndef XTF_ARM_HYPERCALL_H
#define XTF_ARM_HYPERCALL_H

#include <xtf/lib.h>

#define _hypercall_1(type, hcall, a1)                   \
    ({                                                  \
        UNIMPLEMENTED();                                \
        (type)0;                                        \
    })

#define _hypercall_2(type, hcall, a1, a2)               \
    ({                                                  \
        UNIMPLEMENTED();                                \
        (type)0;                                        \
    })

#define _hypercall_3(type, hcall, a1, a2, a3)           \
    ({                                                  \
        UNIMPLEMENTED();                                \
        (type)0;                                        \
    })

#define _hypercall_5(type, hcall, a1, a2, a3, a4, a5)  \
    ({                                                 \
        UNIMPLEMENTED();                               \
        (type)0;                                       \
    })

#endif /* XTF_ARM_HYPERCALL_H */

/*
 * Local variables:
 * mode: C
 * c-file-style: "BSD"
 * c-basic-offset: 4
 * tab-width: 4
 * indent-tabs-mode: nil
 * End:
 */
