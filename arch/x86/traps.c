#include <xtf/traps.h>

/*
 * Getting called means that a shutdown(crash) hypercall has not succeeded.
 * Attempt more extreme measures to try and force a crash, and fall into an
 * infinite loop if all else fails.
 */
void __noreturn arch_crash_hard(void)
{
#if defined(CONFIG_ENV_pv32)
    /*
     * 32bit PV - put the stack in the Xen read-only M2P mappings and attempt
     * to use it.
     */
    asm volatile("mov %0, %%esp; pushf"
                 :: "i" (0xfbadc0deUL) : "memory");

#elif defined(CONFIG_ENV_pv64)
    /*
     * 64bit PV - put the stack in the middle of the non-canonical region, and
     * attempt to use it.
     */
    asm volatile("movabs %0, %%rsp; pushf"
                 :: "i" (0x800000000badc0deUL) : "memory");

#elif defined(CONFIG_ENV_hvm)
    /*
     * HVM - clear interrupts and halt.  Xen should catch this condition and
     * shut the VM down.
     */
    asm volatile("cli; hlt");

#endif

    /*
     * Attempt to crash failed.  Give up and sit in a loop.
     */
    asm volatile("1: hlt; rep; nop; jmp 1b" ::: "memory");
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
