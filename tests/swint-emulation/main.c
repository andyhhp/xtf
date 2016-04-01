/**
 * @file tests/swint-emulation/main.c
 * @ref test-swint-emulation - Emulation of software interrupts.
 *
 * @page test-swint-emulation Software Interrupt Emulation
 *
 * Tests for the exception frame generated from emulated software interrupt
 * instructions, in situations which will results in traps and faults.
 *
 * This test requires a debug Xen, booted with `"hvm_fep"` to allow the test
 * to directly invoke the x86 instruction emulator.
 *
 * Each of the following instructions are tested:
 * - `int3`   (`0xcc`)
 * - `int $3` (`0xcd 0x03`)
 * - `icebp`  (`0xf1`)
 * - `int $1` (`0xcd 0x01`)
 * - `into`   (`0xce`)
 *
 * with and without a redundant prefix (address size override specifically, as
 * it has no effect on instructs like these).  Each combination is executed
 * both normally, and via the instruction emulator, to confirm that hardware
 * and the instruction emulator are behaving identically.
 *
 * Injection of software interrupts by Xen is complicated by differing
 * available of hardware support.  Intel VT-x and AMD SVM (both with and
 * without NextRIP support) all provide different levels of hardware support,
 * which affects how much Xen must emulate in software to cause the guest to
 * observe architectural behaviour.
 *
 * Each instruction/prefix combination is executed in the following
 * conditions:
 * - cpl0, all permissions ok - expect traps
 * - cpl0, descriptors not present - expect @#NP faults
 * - cpl3, all permissions ok - expect traps
 * - cpl3, descriptors dpl0 - expect @#GP faults (except `icebp`)
 * - cpl3, descriptors not present - expect @#NP faults
 *
 * Handling of `into` is more complicated.  In 32bit it is tested as normal,
 * but the instruction isn't recognised in 64bit.  Instead, it is just tested
 * to unconditionally generate a @#UD fault.
 *
 * In all cases, the exception frame is verified to be correct.
 *
 * @sa tests/swint-emulation/main.c
 */
#include <xtf.h>

#include <arch/x86/processor.h>
#include <arch/x86/symbolic-const.h>

#include "lowlevel.h"

/** Single stub's worth of information. */
struct single
{
    const char *type;
    void (*fn)(void);
    void *trap, *fault;
};

/** A collection of subs for an instruction. */
struct sequence
{
    const char *name;
    struct single tests[4];
};

/** Sequence for `int3`. */
struct sequence int3 =
{ "int3",
  {
      {"regular", stub_int3_reg,
       label_int3_reg_trap, label_int3_reg_fault},

      {"redundant", stub_int3_red,
       label_int3_red_trap, label_int3_red_fault},

      {"forced", stub_int3_force,
       label_int3_force_trap, label_int3_force_fault},

      {"forced redundant", stub_int3_forcered,
       label_int3_forcered_trap, label_int3_forcered_fault},
  },
};

/** Sequence for `int $3`. */
struct sequence int_0x3 =
{ "int $3",
  {
      {"regular", stub_int_0x3_reg,
       label_int_0x3_reg_trap, label_int_0x3_reg_fault},

      {"redundant", stub_int_0x3_red,
       label_int_0x3_red_trap, label_int_0x3_red_fault},

      {"forced", stub_int_0x3_force,
       label_int_0x3_force_trap, label_int_0x3_force_fault},

      {"forced redundant", stub_int_0x3_forcered,
       label_int_0x3_forcered_trap, label_int_0x3_forcered_fault},
  },
};

/** Sequence for `icebp`. */
struct sequence icebp =
{ "icebp",
  {
      {"regular", stub_icebp_reg,
       label_icebp_reg_trap, label_icebp_reg_fault},

      {"redundant", stub_icebp_red,
       label_icebp_red_trap, label_icebp_red_fault},

      {"forced", stub_icebp_force,
       label_icebp_force_trap, label_icebp_force_fault},

      {"forced redundant", stub_icebp_forcered,
       label_icebp_forcered_trap, label_icebp_forcered_fault},
  },
};

/** Sequence for `int $1`. */
struct sequence int_0x1 =
{ "int $1",
  {
      {"regular", stub_int_0x1_reg,
       label_int_0x1_reg_trap, label_int_0x1_reg_fault},

      {"redundant", stub_int_0x1_red,
       label_int_0x1_red_trap, label_int_0x1_red_fault},

      {"forced", stub_int_0x1_force,
       label_int_0x1_force_trap, label_int_0x1_force_fault},

      {"forced redundant", stub_int_0x1_forcered,
       label_int_0x1_forcered_trap, label_int_0x1_forcered_fault},
  },
};

/** Sequence for `into`. */
struct sequence into =
{ "into",
  {
      {"regular", stub_into_reg,
       label_into_reg_trap, label_into_reg_fault},

      {"redundant", stub_into_red,
       label_into_red_trap, label_into_red_fault},

      {"forced", stub_into_force,
       label_into_force_trap, label_into_force_fault},

      {"forced redundant", stub_into_forcered,
       label_into_forcered_trap, label_into_forcered_fault},
  },
};

/** Whether to run the stub in user or supervisor mode. */
static bool user = false;

struct expectation {
    const char *prefix;
    const void *ip;
    unsigned int ev, ec;
} /** Expected %%eip, vector and error code from the stub under test. */
    expectation;

/** Latch details of the stub under test. */
void expect(const void *prefix, const void *ip,
            unsigned int ev, unsigned int ec)
{
    expectation = (struct expectation){prefix, ip, ev, ec};
    xtf_exlog_reset();
}

/** Check the exception long against the expected details. */
void check(void)
{
    unsigned int entries = xtf_exlog_entries();

    if ( entries != 1 )
    {
        xtf_failure("Fail %s: Expected 1 exception (vec %u at %p), got %u\n",
                    expectation.prefix, expectation.ev,
                    expectation.ip, entries);
        xtf_exlog_dump_log();
        return;
    }

    exlog_entry_t *entry = xtf_exlog_entry(0);
    if ( !entry )
    {
        xtf_failure("Fail %s: Unable to retrieve exception log\n",
                    expectation.prefix);
        return;
    }

    if ( (_p(entry->ip) != expectation.ip) ||
         (entry->ev != expectation.ev) ||
         (entry->ec != expectation.ec) )
    {
        xtf_failure("Fail %s:\n"
                    "  Expected vec %2u[%04x] at %p\n"
                    "       got vec %2u[%04x] at %p\n",
                    expectation.prefix,
                    expectation.ev, expectation.ec, expectation.ip,
                    entry->ev, entry->ec, _p(entry->ip));
        return;
    }
}

/** Print expected information in the case of an unexpected exception. */
bool unhandled_exception(struct cpu_regs *regs)
{
    printk("Unhandled Exception at %p\n", _p(regs));
    check();

    return false;
}

/** Test a single sequence of related instructions. */
void test_seq(struct sequence *seq, unsigned int vector,
              unsigned int error, bool fault)
{
    unsigned int i;

    printk("  Testing %s\n", seq->name);

    for ( i = 0; i < ARRAY_SIZE(seq->tests); ++i )
    {
        struct single *s = &seq->tests[i];

        expect(s->type,
               fault ? s->fault : s->trap,
               vector, error);

        user ? exec_user(s->fn) : s->fn();

        check();

        /* Avoid 'force' and 'forcered' stubs if FEP isn't available. */
        if ( i == 1 && !xtf_has_fep )
            break;
    }
}

/** test_seq() wrapper, for caller clarity. */
static void test_trap(struct sequence *seq, unsigned int vector)
{
    test_seq(seq, vector, 0, false);
}

/** test_seq() wrapper, for caller clarity. */
static void test_fault(struct sequence *seq,
                       unsigned int vector, unsigned int error)
{
    test_seq(seq, vector, error, true);
}

/** Modify the present flag on the IDT entries under test. */
static void set_idt_entries_present(bool present)
{
    idt[X86_EXC_DB].p = present;
    idt[X86_EXC_BP].p = present;
    idt[X86_EXC_OF].p = present;
}

/** Modify the descriptor privilege level on the IDT entries under test. */
static void set_idt_entries_dpl(unsigned int dpl)
{
    idt[X86_EXC_DB].dpl = dpl;
    idt[X86_EXC_BP].dpl = dpl;
    idt[X86_EXC_OF].dpl = dpl;
}

/** Tests run in user mode. */
void cpl3_tests(void)
{
    user = true;

    printk("Test cpl3: all perms ok\n");
    {
        test_trap(&int3,    X86_EXC_BP);
        test_trap(&int_0x3, X86_EXC_BP);
        test_trap(&icebp,   X86_EXC_DB);
        test_trap(&int_0x1, X86_EXC_DB);
#ifdef __i386__
        test_trap(&into,    X86_EXC_OF);
#else
        test_fault(&into,   X86_EXC_UD, 0);
#endif
    }

    printk("Test cpl3: p=0\n");
    {
        set_idt_entries_present(false);

        test_fault(&int3,    X86_EXC_NP, EXC_EC_SYM(BP));
        test_fault(&int_0x3, X86_EXC_NP, EXC_EC_SYM(BP));
        test_fault(&icebp,   X86_EXC_NP, EXC_EC_SYM(DB, EXT));
        test_fault(&int_0x1, X86_EXC_NP, EXC_EC_SYM(DB));
#ifdef __i386__
        test_fault(&into,    X86_EXC_NP, EXC_EC_SYM(OF));
#else
        test_fault(&into,    X86_EXC_UD, 0);
#endif

        set_idt_entries_present(true);
    }

    printk("Test cpl3: dpl=0\n");
    {
        set_idt_entries_dpl(0);

        test_fault(&int3,    X86_EXC_GP, EXC_EC_SYM(BP));
        test_fault(&int_0x3, X86_EXC_GP, EXC_EC_SYM(BP));
         /* icebp count as external, so no dpl check. */
        test_trap (&icebp,   X86_EXC_DB);
        test_fault(&int_0x1, X86_EXC_GP, EXC_EC_SYM(DB));
#ifdef __i386__
        test_fault(&into,    X86_EXC_GP, EXC_EC_SYM(OF));
#else
        test_fault(&into,    X86_EXC_UD, 0);
#endif

        set_idt_entries_dpl(3);
    }

    user = false;
}

/** Tests run in supervisor mode. */
void cpl0_tests(void)
{
    printk("Test cpl0: all perms ok\n");
    {
        test_trap(&int3,    X86_EXC_BP);
        test_trap(&int_0x3, X86_EXC_BP);
        test_trap(&icebp,   X86_EXC_DB);
        test_trap(&int_0x1, X86_EXC_DB);
#ifdef __i386__
        test_trap(&into,    X86_EXC_OF);
#else
        test_fault(&into,   X86_EXC_UD, 0);
#endif
    }

    printk("Test cpl0: p=0\n");
    {
        set_idt_entries_present(false);

        test_fault(&int3,    X86_EXC_NP, EXC_EC_SYM(BP));
        test_fault(&int_0x3, X86_EXC_NP, EXC_EC_SYM(BP));
        test_fault(&icebp,   X86_EXC_NP, EXC_EC_SYM(DB, EXT));
        test_fault(&int_0x1, X86_EXC_NP, EXC_EC_SYM(DB));
#ifdef __i386__
        test_fault(&into,    X86_EXC_NP, EXC_EC_SYM(OF));
#else
        test_fault(&into,    X86_EXC_UD, 0);
#endif

        set_idt_entries_present(true);
    }
}

void test_main(void)
{
    printk("Trap emulation\n");

    if ( !xtf_has_fep )
        xtf_warning("Warning: FEP support not detected - some tests will be skipped\n");

    /* Setup.  Hook unhandled exceptions for debugging purposes. */
    xtf_unhandled_exception_hook = unhandled_exception;
    set_idt_entries_present(true);
    set_idt_entries_dpl(3);

    xtf_exlog_start();

    cpl0_tests();
    cpl3_tests();

    xtf_exlog_stop();

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
