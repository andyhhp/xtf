#ifndef XTF_BARRIER_H
#define XTF_BARRIER_H

#if defined(__x86_64__) || defined (__i386__)
# include <arch/x86/barrier.h>
#else
# error Bad architecture
#endif

#endif /* XTF_BARRIER_H */

/*
 * Local variables:
 * mode: C
 * c-file-style: "BSD"
 * c-basic-offset: 4
 * tab-width: 4
 * indent-tabs-mode: nil
 * End:
 */
