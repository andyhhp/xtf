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
