/**
 * @file tests/pv-iopl/main.c
 * @ref test-pv-iopl - PV IOPL Emulation
 *
 * @page test-pv-iopl PV IOPL Emulation
 *
 * Tests for the behaviour of Xen's virtual IOPL handling for PV guests.
 *
 * Xen cannot actually run PV guests with an IOPL other than 0, or PV guests
 * would be able to play with the real interrupt flag behind Xen's back.
 * Therefore, the guests desired IOPL is shadowed, and consulted when a IO
 * related fault occurs.  Xen either completes the IO on behalf of the guest,
 * or bounces the @#GP fault back to the guest kernel.
 *
 * The instructions:
 * - `cli`
 * - `outb %al, $0x80`
 *
 * are tested in both user and kernel context, while varying IOPL, and
 * verifying that a @#GP fault was correctly (or not) delivered.
 *
 * Methods of varying IOPL:
 * 1. PHYSDEVOP_set_iopl
 *
 *    The PHYSDEVOP_set_iopl has existed in the Xen ABI for a very long time.
 *    Kernel context is considered to have a cpl of 1, even for 64bit PV
 *    guests executing in ring3.  A side effect of this is that a PV guest
 *    kernel will suffer faults from IO accesses until it sets an IOPL of 1.
 *    A guest may set the current IOPL, but cannot query the value.
 *
 * 2. VMASST_TYPE_architectural_iopl
 *
 *    This is a change to the ABI, introduced in Xen 4.7 (following XSA-171)
 *    to make IOPL handling easier for the guest kernel.  A guest must opt in
 *    to the new ABI by enabling the VMASSIT.  This ABI considers a guest
 *    kernel to have an cpl of 0, and the shadowed IOPL will be updated from
 *    the contents of IRET frame during an IRET hypercall.
 *
 * @see tests/pv-iopl/main.c
 */
#include <xtf.h>

const char test_title[] = "PV IOPL emulation";

static unsigned long stub_cli(void)
{
    unsigned long fault = 0;

    asm ("1: cli; 2:"
         _ASM_EXTABLE_HANDLER(1b, 2b, %P[rec])
         : "+a" (fault)
         : [rec] "p" (ex_record_fault_eax));

    return fault;
}

static unsigned long __user_text stub_user_cli(void)
{
    unsigned long fault = 0;

    asm ("1: cli; 2:"
         _ASM_EXTABLE_HANDLER(1b, 2b, %P[rec])
         : "+a" (fault)
         : [rec] "p" (ex_record_fault_eax));

    return fault;
}

static unsigned long stub_outb(void)
{
    unsigned long fault = 0;

    asm ("1: outb %b0, $0x80; 2:"
         _ASM_EXTABLE_HANDLER(1b, 2b, %P[rec])
         : "+a" (fault) /* Overloaded as the input to OUTB */
         : [rec] "p" (ex_record_fault_eax));

    return fault;
}

static unsigned long __user_text stub_user_outb(void)
{
    unsigned long fault = 0;

    asm ("1: outb %b0, $0x80; 2:"
         _ASM_EXTABLE_HANDLER(1b, 2b, %P[rec])
         : "+a" (fault) /* Overloaded as the input to OUTB */
         : [rec] "p" (ex_record_fault_eax));

    return fault;
}

static const struct insn {
    const char *name;
    unsigned long (*fn)(void);
    unsigned long (*user_fn)(void);
} insns[] = {
    { "cli",  stub_cli,  stub_user_cli,  },
    { "outb", stub_outb, stub_user_outb, },
};

enum mode { KERN, USER };
struct test {
    void (*set_iopl)(unsigned int iopl);
    bool (*should_fault)(enum mode, unsigned int iopl);
};

static void run_test(const struct test *t)
{
    for ( unsigned int iopl = 0; iopl <= 3; ++iopl )
    {
        /* vIOPL 2 is not interesting to test. */
        if ( iopl == 2 )
            continue;

        printk("  vIOPL %u\n", iopl);
        t->set_iopl(iopl);

        for ( unsigned int i = 0; i < ARRAY_SIZE(insns); ++i )
        {
            const struct insn *insn = &insns[i];
            exinfo_t exp, got;

            /* Run insn in kernel. */
            exp = t->should_fault(KERN, iopl) ? EXINFO_SYM(GP, 0) : 0;
            got = insn->fn();

            if ( exp != got )
                xtf_failure("Fail: kern %s, expected %pe, got %pe\n",
                            insn->name, _p(exp), _p(got));

            /* Run insn in userspace. */
            exp = t->should_fault(USER, iopl) ? EXINFO_SYM(GP, 0) : 0;
            got = exec_user(insn->user_fn);

            if ( exp != got )
                xtf_failure("Fail: user %s, expected %pe, got %pe\n",
                            insn->name, _p(exp), _p(got));
        }
    }
}

static void hypercall_set_iopl(unsigned int iopl)
{
    hypercall_physdev_op(PHYSDEVOP_set_iopl, &iopl);
}

static bool hypercall_should_fault(enum mode mode, unsigned int iopl)
{
    /*
     * Kernel has vCPL 1, userspace has vCPL 3
     */
    switch ( iopl )
    {
    case 0:
        /* Both kernel and userspace should fault. */
        return true;

    case 1:
    case 2:
        /* Kernel should succeed, user should fault. */
        return mode == USER;

    case 3:
        /* Both kernel and userspace should succeed. */
        return false;

    default:
        panic("Bad vIOPL %u\n", iopl);
    }
}

static const struct test hypercall = {
    .set_iopl     = hypercall_set_iopl,
    .should_fault = hypercall_should_fault,
};

static void __user_text nop(void) {}
static void vmassist_set_iopl(unsigned int iopl)
{
    /*
     * All that needs to happen to set iopl is to execute an iret hypercall
     * with the appropriate iopl set.  Reuse the exec_user infrastructure to
     * issue the iret, and execute nothing interesting in user context.
     */
    exec_user_efl_or_mask = MASK_INSR(iopl, X86_EFLAGS_IOPL);
    exec_user_void(nop);
}

static bool vmassist_should_fault(enum mode mode, unsigned int iopl)
{
    /*
     * Kernel has vCPL 0, userspace has vCPL 3.
     *
     * Kenrel should never fault, while userspace should only not fault at
     * iopl 3.
     */
    if ( mode == KERN )
        return false;

    return iopl != 3;
}

static const struct test vmassist = {
    .set_iopl     = vmassist_set_iopl,
    .should_fault = vmassist_should_fault,
};

void test_main(void)
{
    const char *cmdline = (const char *)pv_start_info->cmd_line;
    const struct test *test;

    /**
     * @todo Implement better command line infrastructure, but this will do
     * for now.
     */
    if ( !strcmp(cmdline, "hypercall") )
    {
        printk("Test: PHYSDEVOP_set_iopl\n");
        test = &hypercall;
    }
    else if ( !strcmp(cmdline, "vmassist") )
    {
        if ( hypercall_vm_assist(VMASST_CMD_enable,
                                 VMASST_TYPE_architectural_iopl) )
            return xtf_skip("VMASST_TYPE_architectural_iopl not detected\n");

        printk("Test: VMASST_TYPE_architectural_iopl\n");
        test = &vmassist;
    }
    else
        return xtf_error("Unknown test to run\n");

    /* Run the chosen test. */
    run_test(test);
    xtf_success(NULL);
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
