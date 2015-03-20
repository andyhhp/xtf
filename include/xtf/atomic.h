#ifndef XTF_ATOMIC_H
#define XTF_ATOMIC_H

#include <xtf/lib.h>
#include <xtf/barrier.h>

#define LOAD_ACQUIRE(p)                         \
    ({ typeof(*p) _p = ACCESS_ONCE(*p);         \
        smp_rmb();                              \
        _p;                                     \
    })

#define STORE_RELEASE(p, v)                     \
    ({ smp_wmb();                               \
        ACCESS_ONCE(*p) = v;                    \
    })

#endif /* XTF_ATOMIC_H */

/*
 * Local variables:
 * mode: C
 * c-file-style: "BSD"
 * c-basic-offset: 4
 * tab-width: 4
 * indent-tabs-mode: nil
 * End:
 */
