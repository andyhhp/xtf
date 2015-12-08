#include <xtf/test.h>
#include <xtf/report.h>
#include <xtf/console.h>

#include <xtf/traps.h>

static void test_int3_breakpoint(void)
{
    printk("Test: int3 breakpoint\n");

    /*
     * Check that a breakpoint returns normally from the trap handler.
     */
    asm volatile ("int3");
}

void test_main(void)
{
    printk("XTF Selftests\n");

    test_int3_breakpoint();

    xtf_success();
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
