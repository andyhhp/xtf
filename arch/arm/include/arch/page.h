/**
 * @file arch/arm/include/arch/page.h
 */
#ifndef XTF_ARM_PAGE_H
#define XTF_ARM_PAGE_H

#include <xtf/numbers.h>

/* 4kB pages */
#define PAGE_SHIFT      12
#define PAGE_SIZE       (_AC(1, L) << PAGE_SHIFT)
#define PAGE_MASK       (~(PAGE_SIZE - 1))

#define STACK_ORDER       2
#define STACK_SIZE        (PAGE_SIZE << STACK_ORDER)

/* Attribute Indexes */
#define MT_DEVICE_nGnRnE  0x0
#define MT_NORMAL_NC      0x1
#define MT_NORMAL_WT      0x2
#define MT_NORMAL_WB      0x3
#define MT_DEVICE_nGnRE   0x4
#define MT_NORMAL         0x7

/* LPAE Memory region attributes */
#define MAIR0(attr, mt)   ((attr) << ((mt) * 8))
#define MAIR1(attr, mt)   ((attr) << (((mt) * 8) - 32))

#define MAIR0VAL          (MAIR0(0x00, MT_DEVICE_nGnRnE)| \
                           MAIR0(0x44, MT_NORMAL_NC)    | \
                           MAIR0(0xaa, MT_NORMAL_WT)    | \
                           MAIR0(0xee, MT_NORMAL_WB))

#define MAIR1VAL          (MAIR1(0x04, MT_DEVICE_nGnRE) | \
                           MAIR1(0xff, MT_NORMAL))

#define MAIRVAL           (MAIR1VAL << 32 | MAIR0VAL)

/* SCTLR_EL1 */
#define SCTLR_M           (1 << 0)
#define SCTLR_C           (1 << 2)


#endif /* XTF_ARM_PAGE_H */

/*
 * Local variables:
 * mode: C
 * c-file-style: "BSD"
 * c-basic-offset: 4
 * tab-width: 4
 * indent-tabs-mode: nil
 * End:
 */
