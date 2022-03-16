/**
 * @file arch/arm/mm.c
 *
 * Memory management on arm.
 */

#include <xtf/lib.h>
#include <arch/mm.h>

/*
 * Static boot page tables used before BSS is zeroed.
 * Make boot page tables part of the loaded image by putting them inside
 * ".data.page_aligned" so that they are zeroed when loading image into memory.
 */
uint64_t __aligned(PAGE_SIZE) __section(".data.page_aligned") l1_bpgtable[512];
uint64_t __aligned(PAGE_SIZE) __section(".data.page_aligned") l2_bpgtable[512];
uint64_t __aligned(PAGE_SIZE) __section(".data.page_aligned") l1_idmap[512];
uint64_t __aligned(PAGE_SIZE) __section(".data.page_aligned") fix_pgtable[512];

/*
 * Local variables:
 * mode: C
 * c-file-style: "BSD"
 * c-basic-offset: 4
 * tab-width: 4
 * indent-tabs-mode: nil
 * End:
 */
