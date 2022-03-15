/**
 * @file arch/arm/setup.c
 *
 * Early bringup code for arm.
 */
#include <xtf/lib.h>

/* Structure to store boot arguments. */
struct init_data
{
    uint64_t phys_offset;
    void *fdt;
} boot_data;

const char environment_description[] = ENVIRONMENT_DESCRIPTION;

/*
 * Local variables:
 * mode: C
 * c-file-style: "BSD"
 * c-basic-offset: 4
 * tab-width: 4
 * indent-tabs-mode: nil
 * End:
 */
