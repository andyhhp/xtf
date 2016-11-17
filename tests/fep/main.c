/**
 * @file tests/fep/main.c
 * @ref test-fep
 *
 * @page test-fep FEP
 *
 * Test the availability of HVM Forced Emulation Prefix (FEP), which
 * allows HVM guest arbitrarily exercise the instruction emulator.
 *
 * Returns SUCCESS if FEP is available, FAILURE if not.
 *
 * @see tests/fep/main.c
 */
#include <xtf.h>

const char test_title[] = "Test availability of HVM forced emulation prefix";

void test_main(void)
{
    if ( xtf_has_fep )
        xtf_success(NULL);
    else
        xtf_failure(NULL);
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
