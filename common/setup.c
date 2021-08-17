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

/**
 * Some basic assertions about the compile environment.
 */
static void __maybe_unused build_assertions(void)
{
    BUILD_BUG_ON(BITS_PER_LONG != 32 &&
                 BITS_PER_LONG != 64);
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
