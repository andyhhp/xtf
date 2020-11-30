#include <xtf/asm_macros.h>

/*
 * flush_dcache_range(start, end)
 * - start   - start address of a region
 * - end     - end address of a region
 */
ENTRY(flush_dcache_range)
    mov    x5, x0
    /* Get the minimum D-cache line size */
    mrs    x3, ctr_el0
    ubfm   x3, x3, #16, #19
    mov    x2, #4
    lsl    x2, x2, x3
    sub    x3, x2, #1
    bic    x5, x5, x3
    /* Clean and invalidate D-cache line */
1:  dc     civac, x5
    add    x5, x5, x2
    cmp    x5, x1
    b.lo   1b
    dsb    sy
    ret
ENDFUNC(flush_dcache_range)
