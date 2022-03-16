/**
 * @file arch/arm/include/arch/mm.h
 *
 * Memory management on arm.
 */
#ifndef XTF_ARM_MM_H
#define XTF_ARM_MM_H

#include <arch/page.h>
#include <xtf/types.h>

/*
 * Granularity: 4KB
 * VA width: 39bit
 * Tables: L1, L2, L3(fixmap)
 */
#define VA_WIDTH                39
#define SZ_2M                   0x200000
#define VA_LIMIT                0xFFFFFFFFFFFFFFFF
#define VA_START                (VA_LIMIT << VA_WIDTH)
#define PAGE_OFFSET             (VA_LIMIT << (VA_WIDTH - 1))
#define TABLE_ENTRIES           512
#define TABLE_ADDR_MASK         (TABLE_ENTRIES -1)
#define FIXMAP_ADDR(n)          (VA_START +  SZ_2M + n * PAGE_SIZE)

/*
 * L1 translation table
 * 1 entry = 1GB
 */
#define L1_TABLE_SHIFT          30
#define L1_TABLE_SIZE           (1 << L1_TABLE_SHIFT)
#define L1_TABLE_OFFSET         (L1_TABLE_SIZE - 1)
#define L1_TABLE_INDEX(x)       ((x >> L1_TABLE_SHIFT) & TABLE_ADDR_MASK)

/*
 * L2 translation table
 * 1 entry = 2MB
 */
#define L2_TABLE_SHIFT          21
#define L2_TABLE_SIZE           (1 << L2_TABLE_SHIFT)
#define L2_TABLE_OFFSET         (L2_TABLE_SIZE - 1)
#define L2_TABLE_INDEX(x)       ((x >> L2_TABLE_SHIFT) & TABLE_ADDR_MASK)

/*
 * L3 translation table
 * 1 entry = 4KB
 */
#define L3_TABLE_SHIFT          PAGE_SHIFT
#define L3_TABLE_SIZE           (1 << L3_TABLE_SHIFT)
#define L3_TABLE_OFFSET         (L3_TABLE_SIZE - 1)
#define L3_TABLE_INDEX(x)       ((x >> L3_TABLE_SHIFT) & TABLE_ADDR_MASK)

/* Fixmap slots */
#define FIXMAP_PV_CONSOLE       0

/* Descriptors */
#define DESCR_BAD               0x0
#define DESCR_VALID             0x1
#define DESC_TYPE_TABLE         (0x1 << 1)
#define DESC_TYPE_BLOCK         (0x0 << 1)
#define DESC_MAIR_INDEX(x)      (x << 2)
#define DESC_NS(x)              (x << 5)
#define DESC_AP(x)              (x << 6)
#define DESC_SH(x)              (x << 8)
#define DESC_AF(x)              (x << 10)
#define DESC_PXN(x)             (x << 53)
#define DESC_UXN(x)             (x << 54)

#define DESC_PAGE_TABLE         (DESCR_VALID | DESC_TYPE_TABLE)

#define DESC_PAGE_BLOCK         (DESCR_VALID | DESC_TYPE_BLOCK |\
                                 DESC_MAIR_INDEX(MT_NORMAL) |\
                                 DESC_AF(0x1) | DESC_SH(0x3))

#define DESC_PAGE_TABLE_DEV     (DESCR_VALID | DESC_TYPE_TABLE |\
                                 DESC_MAIR_INDEX(MT_DEVICE_nGnRnE) |\
                                 DESC_AF(0x1) | DESC_SH(0x3))

#ifndef __ASSEMBLY__
typedef uint64_t paddr_t;
extern paddr_t phys_offset;

/*
 * PFN - physical frame number
 * MFN - machine frame number
 * PO  - physical offset
 * PA  - physical address
 * VA  - virtual address
 *
 * PA = PO + VA
 * VA = PA - PO
 */
#define phys(x)         ((paddr_t)(x) + phys_offset)
#define virt(x)         (void *)(((x) - phys_offset)
#define pfn_to_phys(x)  ((paddr_t)(x) << PAGE_SHIFT)
#define phys_to_pfn(x)  ((unsigned long)((x) >> PAGE_SHIFT))
#define mfn_to_virt(x)  (virt(pfn_to_phys(x)))
#define virt_to_mfn(x)  (phys_to_pfn(phys(x)))
#define pfn_to_virt(x)  (virt(pfn_to_phys(x)))
#define virt_to_pfn(x)  (phys_to_pfn(phys(x)))

void store_pgt_entry(uint64_t *addr, uint64_t val);
uint64_t set_fixmap(uint8_t slot, paddr_t pa, uint64_t flags);
void setup_mm(paddr_t boot_phys_offset);

#endif /* __ASSEMBLY__ */

#endif /* XTF_ARM_MM_H */

/*
 * Local variables:
 * mode: C
 * c-file-style: "BSD"
 * c-basic-offset: 4
 * tab-width: 4
 * indent-tabs-mode: nil
 * End:
 */
