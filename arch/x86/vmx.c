/**
 * @file arch/x86/vmx.c
 *
 * Helpers for VT-x.
 */
#include <xtf/lib.h>

#include <arch/vmx.h>

const char *vmx_insn_err_strerror(unsigned int err)
{
#define ERR(x) [VMERR_ ## x] = #x
    static const char *const strings[] = {
        ERR(VMCALL_IN_ROOT),
        ERR(VMCLEAR_BAD_PADDR),
        ERR(VMCLEAR_WITH_VMXON_PTR),
        ERR(VMLAUNCH_NONCLEAR_VMCS),
        ERR(VMRESUME_NONLAUNCHED_VMCS),
        ERR(VMRESUME_AFTER_VMXOFF),
        ERR(INVALID_CONTROL_STATE),
        ERR(INVALID_HOST_STATE),
        ERR(VMPTRLD_BAD_PADDR),
        ERR(VMPTRLD_WITH_VMXON_PTR),
        ERR(VMPTRLD_BAD_REVID),
        ERR(UNSUPPORTED_VMCS_FIELD),
        ERR(VMWRITE_READONLY_FIELD),
        ERR(VMXON_IN_ROOT),
        ERR(VMENTRY_BAD_EXECUTIVE),
        ERR(VMENTRY_NONLAUNCHED_EXECUTIVE),
        ERR(VMENTRY_WITHOUT_VMXON_PTR),
        ERR(VMCALL_NONCLEAR_VMCS),
        ERR(VMCALL_BAD_CONTROL_STATE),
        ERR(VMCALL_BAD_MSEG_REVID),
        ERR(VMXOFF_UNDER_DUAL),
        ERR(VMCALL_BAD_SMM_MONITOR),
        ERR(BAD_EXECUTIVE_STATE),
        ERR(VMENTRY_MOVSS),
        ERR(BAD_INV_OPERAND),
    };
#undef ERR

    if ( err < ARRAY_SIZE(strings) && strings[err] )
        return strings[err];
    else
        return "<unknown>";
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
