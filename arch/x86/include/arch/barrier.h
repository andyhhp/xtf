#ifndef XTF_X86_BARRIER_H
#define XTF_X86_BARRIER_H

/*
 * Memory barriers for x86 systems
 *
 * See Linux: Documentation/memory-barriers.txt for a very detailed
 * description of the problems and their implications.
 *
 * Under Xen, we rely on the fact that only x86_64 cpus are supported, which
 * guarantees that the {m,l,s}fence instructions are supported (SSE2 being a
 * requirement of 64bit).
 *
 * x86 memory ordering requirements make the smp_???() variants easy.  From
 * the point of view of program order, reads may not be reordered with respect
 * to other reads, and writes may not be reordered with respect to other
 * writes, causing smp_rmb() and smp_wmb() to degrade to simple compiler
 * barriers.  smp_mb() however does need to be an mfence instruction, as reads
 * are permitted to be reordered ahead of non-aliasing writes.
 */

#include <xtf/compiler.h>

#define mb()      __asm__ __volatile__ ("mfence" ::: "memory")
#define rmb()     __asm__ __volatile__ ("lfence" ::: "memory")
#define wmb()     __asm__ __volatile__ ("sfence" ::: "memory")

#define smp_mb()  mb()
#define smp_rmb() barrier()
#define smp_wmb() barrier()

#endif /* XTF_X86_BARRIER_H */

/*
 * Local variables:
 * mode: C
 * c-file-style: "BSD"
 * c-basic-offset: 4
 * tab-width: 4
 * indent-tabs-mode: nil
 * End:
 */
