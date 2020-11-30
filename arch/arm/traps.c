/**
 * @file arch/arm/traps.c
 *
 * Arm trap handlers.
 */
#include <xtf/traps.h>
#include <xtf/lib.h>

void __noreturn arch_crash_hard(void)
{
    /* Sit in the infinite loop */
    while (1);
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
