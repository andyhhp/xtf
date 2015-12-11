#include <xtf/traps.h>

void arch_init_traps(void)
{
}

void __noreturn arch_crash_hard(void)
{
    /*
     * Clear interrupts and halt.  Xen should catch this condition and shut
     * the VM down.  If that fails, sit in a loop.
     */
    asm volatile("cli;"
                 "1: hlt;"
                 "pause;"
                 "jmp 1b"
                 ::: "memory");
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
