/**
 * @file arch/x86/include/arch/x86-vmx.h
 *
 * VT-x hardware ABI, as specified in the Intel SDM.
 */
#ifndef XTF_X86_X86_VMX_H
#define XTF_X86_X86_VMX_H

/* VMX Instruction Error codes. */
#define VMERR_VMCALL_IN_ROOT                         1
#define VMERR_VMCLEAR_BAD_PADDR                      2
#define VMERR_VMCLEAR_WITH_VMXON_PTR                 3
#define VMERR_VMLAUNCH_NONCLEAR_VMCS                 4
#define VMERR_VMRESUME_NONLAUNCHED_VMCS              5
#define VMERR_VMRESUME_AFTER_VMXOFF                  6
#define VMERR_INVALID_CONTROL_STATE                  7
#define VMERR_INVALID_HOST_STATE                     8
#define VMERR_VMPTRLD_BAD_PADDR                      9
#define VMERR_VMPTRLD_WITH_VMXON_PTR                10
#define VMERR_VMPTRLD_BAD_REVID                     11
#define VMERR_UNSUPPORTED_VMCS_FIELD                12
#define VMERR_VMWRITE_READONLY_FIELD                13
/* 14 not specified. */
#define VMERR_VMXON_IN_ROOT                         15
#define VMERR_VMENTRY_BAD_EXECUTIVE                 16
#define VMERR_VMENTRY_NONLAUNCHED_EXECUTIVE         17
#define VMERR_VMENTRY_WITHOUT_VMXON_PTR             18
#define VMERR_VMCALL_NONCLEAR_VMCS                  19
#define VMERR_VMCALL_BAD_CONTROL_STATE              20
/* 21 not specified. */
#define VMERR_VMCALL_BAD_MSEG_REVID                 22
#define VMERR_VMXOFF_UNDER_DUAL                     23
#define VMERR_VMCALL_BAD_SMM_MONITOR                24
#define VMERR_BAD_EXECUTIVE_STATE                   25
#define VMERR_VMENTRY_MOVSS                         26
/* 27 not specified. */
#define VMERR_BAD_INV_OPERAND                       28


/* VMCS field encodings. */
#define VMCS_VM_INSN_ERR                        0x4400

#endif /* XTF_X86_X86_VMX_H */

/*
 * Local variables:
 * mode: C
 * c-file-style: "BSD"
 * c-basic-offset: 4
 * tab-width: 4
 * indent-tabs-mode: nil
 * End:
 */
