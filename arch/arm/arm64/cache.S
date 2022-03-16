#include <xtf/asm_macros.h>

/*
 * clean_and_invalidate_dcache(start, end)
 * - x0(start) - start address of a region
 * - x1(end)   - end address of a region
 * Clobbers: x2, x3, x4
 */
ENTRY(clean_and_invalidate_dcache)
    /* Do not modify x0 */
    mov    x4, x0
    /* Get the minimum D-cache line size */
    mrs    x3, ctr_el0
    ubfm   x3, x3, #16, #19
    mov    x2, #4
    lsl    x2, x2, x3
    sub    x3, x2, #1
    bic    x4, x4, x3
    /* Clean and invalidate D-cache line */
1:  dc     civac, x4
    add    x4, x4, x2
    cmp    x4, x1
    b.lo   1b
    dsb    sy
    ret
ENDFUNC(clean_and_invalidate_dcache)
