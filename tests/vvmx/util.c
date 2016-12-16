#include "test.h"

#include <arch/decode.h>

static void decode_test_exinfo(char *str, size_t n, exinfo_t ex)
{
    if ( ex == VMERR_SUCCESS )
    {
        strcpy(str, "VMsucceed");
        return;
    }

    if ( ex == VMERR_INVALID )
    {
        strcpy(str, "VMfailInvalid");
        return;
    }

    unsigned int high = ex & ~0xffffff;

    if ( high == VMERR_VALID(0) )
    {
        unsigned int low = ex & 0xffffff;

        snprintf(str, n, "VMfailValid(%u) %s",
                 low, vmx_insn_err_strerror(low));
        return;
    }

    if ( high == EXINFO_EXPECTED )
    {
        x86_decode_exinfo(str, n, ex);
        return;
    }

    strcpy(str, "<bad>");
}

void check(const char *func, exinfo_t got, exinfo_t exp)
{
    char gotstr[48], expstr[48];

    if ( got == exp )
        return;

    decode_test_exinfo(gotstr, ARRAY_SIZE(gotstr), got);
    decode_test_exinfo(expstr, ARRAY_SIZE(expstr), exp);

    xtf_failure("Failure in %s()\n"
                "  Expected 0x%08x: %s\n"
                "       Got 0x%08x: %s\n",
                func, exp, expstr, got, gotstr);
}

/*
 * Read the VM Instruction Error code from the VMCS.  It is the callers
 * responsibility to ensure that the VMCS is valid in context.
 */
static exinfo_t get_vmx_insn_err(void)
{
    unsigned long err;

    asm ("vmread %[field], %[value]"
         : [value] "=rm" (err)
         : [field] "r" (VMCS_VM_INSN_ERR + 0ul));

    return VMERR_VALID(err);
}

exinfo_t stub_vmxon(uint64_t paddr)
{
    exinfo_t ex = 0;
    bool fail_valid = false, fail_invalid = false;

    asm volatile ("1: vmxon %[paddr];"
                  ASM_FLAG_OUT(, "setc %[fail_invalid];")
                  ASM_FLAG_OUT(, "setz %[fail_valid];")
                  "2:"
                  _ASM_EXTABLE_HANDLER(1b, 2b, ex_record_fault_edi)
                  : "+D" (ex),
                    ASM_FLAG_OUT("=@ccc", [fail_invalid] "+rm") (fail_invalid),
                    ASM_FLAG_OUT("=@ccz", [fail_valid]   "+rm") (fail_valid)
                  : [paddr] "m" (paddr),
                    "X" (ex_record_fault_edi));

    if ( fail_invalid )
        return VMERR_INVALID;
    else if ( fail_valid )
        return get_vmx_insn_err();
    else
        return ex;
}

exinfo_t __user_text stub_vmxon_user(uint64_t paddr)
{
    exinfo_t ex = 0;
    bool fail_valid = false, fail_invalid = false;

    asm volatile ("1: vmxon %[paddr];"
                  ASM_FLAG_OUT(, "setc %[fail_invalid];")
                  ASM_FLAG_OUT(, "setz %[fail_valid];")
                  "2:"
                  _ASM_EXTABLE_HANDLER(1b, 2b, ex_record_fault_edi)
                  : "+D" (ex),
                    ASM_FLAG_OUT("=@ccc", [fail_invalid] "+rm") (fail_invalid),
                    ASM_FLAG_OUT("=@ccz", [fail_valid]   "+rm") (fail_valid)
                  : [paddr] "m" (paddr),
                    "X" (ex_record_fault_edi));

    if ( fail_invalid )
        return VMERR_INVALID;
    else if ( fail_valid )
        return get_vmx_insn_err();
    else
        return ex;
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
