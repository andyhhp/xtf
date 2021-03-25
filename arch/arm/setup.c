/**
 * @file arch/arm/setup.c
 *
 * Early bringup code for arm.
 */
#include <xtf/hypercall.h>
#include <xtf/framework.h>

const char environment_description[] = ENVIRONMENT_DESCRIPTION;

/* Structure to store boot arguments */
struct init_data
{
    uint64_t phys_offset;
    void *fdt;
} boot_data;

void arch_setup(void)
{
    /* Use Xen console to print messages */
    register_console_callback(hypercall_console_write);
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
