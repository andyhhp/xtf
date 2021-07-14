/**
 * @file arch/arm/setup.c
 *
 * Early bringup code for arm.
 */
#include <xtf/hypercall.h>
#include <xtf/framework.h>
#include <arch/pl011.h>

const char environment_description[] = ENVIRONMENT_DESCRIPTION;

/* Structure to store boot arguments */
struct init_data
{
    uint64_t phys_offset;
    void *fdt;
} boot_data;

void setup_console(void)
{
#ifdef CONFIG_PL011_UART
#ifndef CONFIG_PL011_EARLY_PRINTK
    /* Initialize UART */
    pl011_init();
#endif
    /* Use PL011 UART to print messages */
    register_console_callback(pl011_console_write);
#else
    /* Use Xen console to print messages */
    register_console_callback(hypercall_console_write);
#endif
}

void arch_setup(void)
{
    setup_console();
}

void test_setup(void)
{
    /* Nothing to be done here for now */
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
