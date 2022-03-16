/**
 * @file arch/arm/setup.c
 *
 * Early bringup code for arm.
 */
#include <xtf/lib.h>
#include <xtf/hypercall.h>

/* Structure to store boot arguments. */
struct init_data
{
    uint64_t phys_offset;
    void *fdt;
} boot_data;

const char environment_description[] = ENVIRONMENT_DESCRIPTION;

static void setup_console(void)
{
    /* Use Xen console to print messages */
    register_console_callback(hypercall_console_write);
}

void arch_setup(void)
{
    setup_console();
#ifdef CONFIG_MMU
    setup_mm(boot_data.phys_offset);
#endif
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
