#ifndef XTF_HYPERCALL_H
#define XTF_HYPERCALL_H

#include <xtf/types.h>

#if defined(__x86_64__)

# include <arch/x86/x86_64/hypercall-x86_64.h>
# define HYPERCALL2 _hypercall64_2

#elif defined(__i386__)

# include <arch/x86/x86_32/hypercall-x86_32.h>
# define HYPERCALL2 _hypercall32_2

#else
# error Bad architecture for hypercalls
#endif

/* All Xen ABI for includers convenience .*/
#include <xen/xen.h>
#include <xen/sched.h>

/*
 * Hypercall primatives, compiled for the correct bitness
 */
static inline long hypercall_sched_op(unsigned int cmd, void *arg)
{
    return HYPERCALL2(long, sched_op, cmd, arg);
}

/*
 * Higher level hypercall helpers
 */
static inline long hypercall_shutdown(unsigned int reason)
{
    return hypercall_sched_op(SCHEDOP_shutdown, &reason);
}

#endif /* XTF_HYPERCALL_H */

/*
 * Local variables:
 * mode: C
 * c-file-style: "BSD"
 * c-basic-offset: 4
 * tab-width: 4
 * indent-tabs-mode: nil
 * End:
 */
