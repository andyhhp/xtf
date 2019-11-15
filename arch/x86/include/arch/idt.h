/**
 * @file arch/x86/include/arch/idt.h
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

/**
 * Available for test use.
 */
#define X86_VEC_AVAIL    0x21


#ifndef __ASSEMBLY__

/** A guest agnostic represention of IDT information. */
struct xtf_idte
{
    unsigned long addr;
    unsigned int cs, dpl;
};

/**
 * Set up an IDT Entry, in a guest agnostic way.
 *
 * Construct an IDT Entry at the specified @p vector, using configuration
 * provided in @p idte.
 *
 * @param vector Vector to set up.
 * @param idte Details to set up.
 * @returns 0 for HVM guests, hypercall result for PV guests.
 */
int xtf_set_idte(unsigned int vector,
                 const struct xtf_idte *idte);

#endif /* __ASSEMBLY__ */

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
