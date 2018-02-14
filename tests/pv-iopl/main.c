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
 *    the contents of iret frame during an iret hypercall.
 *
 * @see tests/pv-iopl/main.c
 */
#include <xtf.h>

const char test_title[] = "PV IOPL emulation";

bool test_wants_user_mappings = true;

/**
 * Execute @p fn at user privilege, folding @p iopl into the iret frame.
 */
void exec_user_with_iopl(void (*fn)(void), unsigned int iopl);

/** Stub CLI instruction with @#GP fixup. */
static void stub_cli(void)
{
    asm volatile ("1: cli; 2:"
                  _ASM_EXTABLE(1b, 2b));
}

/** Stub OUTB instruction with @#GP fixup. */
static void stub_outb(void)
{
    asm volatile ("1: outb %b0, $0x80; 2:"
                  _ASM_EXTABLE(1b, 2b)
                  :: "a" (0));
}

static const struct insn
{
    const char *name;
    void (*fn)(void);
} /** Sequence of instructions to run. */
    insn_sequence[] =
{
    { "cli",  stub_cli,  },
    { "outb", stub_outb, },
};

static struct expectation
{
    const char *insn;
    bool user;
    bool fault;
} /** Details about the stub under test. */
expectation;

/** Latch details of the stub under test. */
static void expect(const char *insn, bool user, bool fault)
{
    expectation = (struct expectation){insn, user, fault, };
    xtf_exlog_reset();
}

/** Check the exception long against the expected details. */
static void check(void)
{
    unsigned int entries = xtf_exlog_entries();
    const char *mode = expectation.user ? "user" : "kernel";

    if ( expectation.fault )
    {
        if ( entries != 1 )
        {
            xtf_failure("Fail (%s %s): Expected 1 exception, got %u\n",
                        mode, expectation.insn, entries);
            xtf_exlog_dump_log();
            return;
        }

        exlog_entry_t *entry = xtf_exlog_entry(0);
        if ( !entry )
        {
            xtf_failure("Fail (%s %s): Unable to retrieve exception log\n",
                        mode, expectation.insn);
            return;
        }

        if ( (entry->ev != X86_EXC_GP) || (entry->ec != 0) )
        {
            xtf_failure("Fail (%s %s): Expected #GP[0], got %2u[%04x]\n",
                        mode, expectation.insn, entry->ev, entry->ec);
            return;
        }
    }
    else
    {
        if ( entries != 0 )
        {
            xtf_failure("Fail (%s %s): Expected no exceptions, got %u\n",
                        mode, expectation.insn, entries);
            xtf_exlog_dump_log();
            return;
        }
    }
}

struct test
{
    void (*set_iopl)(unsigned int iopl);
    bool (*should_fault)(bool user, unsigned int iopl);
};

/** Test the instruction sequence using a specific iopl interface. */
static void run_test(const struct test *t)
{
    unsigned int i, iopl;

    xtf_exlog_start();

    for ( iopl = 0; iopl <= 3; ++iopl )
    {
        /* vIOPL 2 is not interesting to test. */
        if ( iopl == 2 )
            continue;

        printk("  vIOPL %u\n", iopl);
        t->set_iopl(iopl);

        for ( i = 0; i < ARRAY_SIZE(insn_sequence); ++i )
        {
            const struct insn *seq = &insn_sequence[i];

            /* Run insn in kernel. */
            expect(seq->name, 0, t->should_fault(0, iopl));
            seq->fn();
            check();

            /* Run insn in userspace. */
            expect(seq->name, 1, t->should_fault(1, iopl));
            exec_user_with_iopl(seq->fn, iopl);
            check();
        }
    }

    xtf_exlog_stop();
}

static void hypercall_set_iopl(unsigned int iopl)
{
    hypercall_physdev_op(PHYSDEVOP_set_iopl, &iopl);
}

static bool hypercall_should_fault(bool user, unsigned int iopl)
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
        return user;

    case 3:
        /* Both kernel and userspace should succeed. */
        return false;

    default:
        panic("Bad vIOPL %u\n", iopl);
    }
}

/** Hypercall based IOPL interface. */
static const struct test hypercall =
{
    .set_iopl     = hypercall_set_iopl,
    .should_fault = hypercall_should_fault,
};

static void nop(void){}
static void vmassist_set_iopl(unsigned int iopl)
{
    /*
     * All that needs to happen to set iopl is to execute an iret hypercall
     * with the appropriate iopl set.  Reuse the exec_user infrastructure to
     * issue the iret, and execute nothing interesting in user context.
     */
    exec_user_with_iopl(nop, iopl);
}

static bool vmassist_should_fault(bool user, unsigned int iopl)
{
    /*
     * Kernel has vCPL 0, userspace has vCPL 3.
     *
     * Kenrel should never fault, while userspace should only not fault at
     * iopl 3.
     */
    if ( !user )
        return false;

    return iopl != 3;
}

/** VMASSIT based IOPL interface. */
static const struct test vmassist =
{
    .set_iopl     = vmassist_set_iopl,
    .should_fault = vmassist_should_fault,
};

void test_main(void)
{
    const struct test *test;

    /**
     * @todo Implement better command line infrastructure, but this will do
     * for now.
     */
    if ( !strcmp((char *)pv_start_info->cmd_line, "hypercall") )
    {
        printk("Test: PHYSDEVOP_set_iopl\n");
        test = &hypercall;
    }
    else if ( !strcmp((char *)pv_start_info->cmd_line, "vmassist") )
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
