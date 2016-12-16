#include "test.h"

/**
 * vmxon with CR4.VMXE cleared
 *
 * Expect: @#UD
 */
static void test_vmxon_novmxe(void)
{
    check(__func__, stub_vmxon(0), EXINFO_SYM(UD, 0));
}

void test_vmxon(void)
{
    unsigned long cr4 = read_cr4();

    if ( cr4 & X86_CR4_VMXE )
        write_cr4(cr4 &= ~X86_CR4_VMXE);

    printk("Test: vmxon\n");

    test_vmxon_novmxe();
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
