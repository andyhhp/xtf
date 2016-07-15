/**
 * @file include/arch/x86/idt.h
 *
 * %x86 IDT vector infrastructure.
 */

#ifndef XTF_X86_IDT_H
#define XTF_X86_IDT_H

/**
 * Return to kernel mode.
 *
 * To enable easy transition between user and kernel mode for tests.
 */
#define X86_VEC_RET2KERN 0x20

#endif /* XTF_X86_IDT_H */

/*
 * Local variables:
 * mode: C
 * c-file-style: "BSD"
 * c-basic-offset: 4
 * tab-width: 4
 * indent-tabs-mode: nil
 * End:
 */
