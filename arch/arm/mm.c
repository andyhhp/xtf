/**
 * @file arch/arm/mm.c
 *
 * Memory management on arm.
 */

#include <xtf/lib.h>
#include <arch/mm.h>
#include <arch/barrier.h>

paddr_t phys_offset;

/*
 * Static boot page tables used before BSS is zeroed.
 * Make boot page tables part of the loaded image by putting them inside
 * ".data.page_aligned" so that they are zeroed when loading image into memory.
 */
uint64_t __aligned(PAGE_SIZE) __section(".data.page_aligned") l1_bpgtable[512];
uint64_t __aligned(PAGE_SIZE) __section(".data.page_aligned") l2_bpgtable[512];
uint64_t __aligned(PAGE_SIZE) __section(".data.page_aligned") l1_idmap[512];
uint64_t __aligned(PAGE_SIZE) __section(".data.page_aligned") fix_pgtable[512];

void store_pgt_entry(uint64_t *addr, uint64_t val)
{
    *addr = val;
    dsb(ishst);
    isb();
}

/* Map a page in a fixmap entry */
uint64_t set_fixmap(uint8_t slot, paddr_t pa, uint64_t flags)
{
    unsigned int index;

    index = L3_TABLE_INDEX(FIXMAP_ADDR(slot));
    store_pgt_entry(&fix_pgtable[index], ((pa & ~(L3_TABLE_SIZE - 1)) | flags));

    return (uint64_t)(FIXMAP_ADDR(slot) + (pa & PAGE_OFFSET));
}

void setup_mm(paddr_t boot_phys_offset)
{
    phys_offset = boot_phys_offset;
}

/*
 * Local variables:
 * mode: C
 * c-file-style: "BSD"
 * c-basic-offset: 4
 * tab-width: 4
 * indent-tabs-mode: nil
 * End:
 */
