/**
 * @file arch/x86/include/arch/vmx.h
 *
 * Helpers for VT-x.
 */
#ifndef XTF_X86_VMX_H
#define XTF_X86_VMX_H

#include <arch/x86-vmx.h>

/**
 * Error string for VMX Instruction Errors.
 */
const char *vmx_insn_err_strerror(unsigned int err);

#endif /* XTF_X86_VMX_H */

/*
 * Local variables:
 * mode: C
 * c-file-style: "BSD"
 * c-basic-offset: 4
 * tab-width: 4
 * indent-tabs-mode: nil
 * End:
 */
