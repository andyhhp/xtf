/**
 * @file common/setup.c
 *
 * C entry point.
 */

#include <xtf/compiler.h>
#include <xtf/hypercall.h>
#include <xtf/framework.h>
#include <xtf/test.h>
#include <xtf/console.h>
#include <xtf/report.h>

/**
 * Entry point into C.
 *
 * Set up the microkernel and invoke the test.  Report the tests status
 * afterwards, and shut down.
 */
void __noreturn xtf_main(void)
{
    arch_setup();

    printk("--- Xen Test Framework ---\n");
    printk("Environment: %s\n", environment_description);
    printk("%s\n", test_title);

    test_setup();

    if ( !xtf_status_reported() )
    {
        test_main();
    }

    /* Report status and exit. */
    xtf_exit();
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
