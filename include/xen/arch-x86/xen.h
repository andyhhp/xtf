/*
 * Xen x86 public interface
 */

#ifndef XEN_PUBLIC_ARCH_X86_XEN_H
#define XEN_PUBLIC_ARCH_X86_XEN_H

#if defined (__i386__)
# include "xen-x86_32.h"
#elif defined (__x86_64__)
# include "xen-x86_64.h"
#else
# error Bad architecture
#endif

#ifndef __ASSEMBLY__
typedef unsigned long xen_pfn_t;
#endif

#endif /* XEN_PUBLIC_ARCH_X86_XEN_H */

/*
 * Local variables:
 * mode: C
 * c-file-style: "BSD"
 * c-basic-offset: 4
 * tab-width: 4
 * indent-tabs-mode: nil
 * End:
 */
