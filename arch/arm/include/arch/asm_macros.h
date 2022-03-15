/**
 * @file arch/arm/include/arch/asm_macros.h
 *
 * Macros for use in arm assembly files.
 */
#ifndef XTF_ARM_ASM_MACROS_H
#define XTF_ARM_ASM_MACROS_H

#define ALIGN .align 2

#ifdef CONFIG_ARM_32
#define __HVC(imm16) .long \
    ((0xE1400070 | (((imm16) & 0xFFF0) << 4) | ((imm16) & 0x000F)) & 0xFFFFFFFF)
#endif

#endif /* XTF_ARM_ASM_MACROS_H */

/*
 * Local variables:
 * mode: C
 * c-file-style: "BSD"
 * c-basic-offset: 4
 * tab-width: 4
 * indent-tabs-mode: nil
 * End:
 */
