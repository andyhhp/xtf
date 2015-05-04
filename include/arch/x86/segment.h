#ifndef XTF_X86_SEGMENT_H
#define XTF_X86_SEGMENT_H

/*
 * GDT layout:
 *
 * For simplicitly, the gdt is shared as much as possible between different
 * environments.
 *
 *  0 - null
 *  1 - 64bit supervisor code
 *  2 - 32bit supervisor code
 *  3 - 32bit supervisor data
 */

#define GDTE_CS64_DPL0 1
#define GDTE_CS32_DPL0 2
#define GDTE_DS32_DPL0 3

#define NR_GDT_ENTRIES 4

#if defined(CONFIG_ENV_hvm64)

#define __KERN_CS (GDTE_CS64_DPL0 * 8)
#define __KERN_DS (0)

#elif defined(CONFIG_ENV_hvm32)

#define __KERN_CS (GDTE_CS32_DPL0 * 8)
#define __KERN_DS (GDTE_DS32_DPL0 * 8)

#endif

#endif /* XTF_X86_SEGMENT_H */

/*
 * Local variables:
 * mode: C
 * c-file-style: "BSD"
 * c-basic-offset: 4
 * tab-width: 4
 * indent-tabs-mode: nil
 * End:
 */
