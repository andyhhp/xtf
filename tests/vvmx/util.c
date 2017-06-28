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
 * Local variables:
 * mode: C
 * c-file-style: "BSD"
 * c-basic-offset: 4
 * tab-width: 4
 * indent-tabs-mode: nil
 * End:
 */
