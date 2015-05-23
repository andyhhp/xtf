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

/**
 * Entry point into C.
 *
 * Set up the microkernel and invoke the test.
 */
void __noreturn xtf_main(void)
{
    arch_setup();

    printk("--- Xen Test Framework ---\n");

    test_main();

    hypercall_shutdown(SHUTDOWN_poweroff);
    unreachable();
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
