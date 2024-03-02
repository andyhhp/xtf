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
 * @see tests/swint-emulation/main.c
 */
#include <xtf.h>

#include "lowlevel.h"

const char test_title[] = "Software interrupt emulation";

bool test_wants_user_mappings = true;

#ifdef __i386__
# define COND(_32, _64) _32
#else
# define COND(_32, _64) _64
#endif

enum mode { KERN, USER };

/** Single stub's worth of information. */
struct single
{
    const char *type;
    void (*fn)(void);
    void *trap, *fault;
};

struct insn
{
    const char *name;
    struct single tests[4];
};

const struct insn int3 = {
    "int3",
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

const struct insn int_0x3 = {
    "int $3",
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

const struct insn icebp = {
    "icebp",
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

const struct insn int_0x1 = {
    "int $1",
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

const struct insn into = {
    "into",
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
bool do_unhandled_exception(struct cpu_regs *regs)
{
    printk("Unhandled Exception at %p\n", _p(regs));
    check();

    return false;
}

void test_insn(enum mode user, const struct insn *insn, exinfo_t exp)
{
    unsigned int vector = exinfo_vec(exp);
    unsigned int error = exinfo_ec(exp);
    bool fault = X86_EXC_FAULTS & (1u << vector);

    printk("  Testing %s\n", insn->name);

    for ( unsigned int i = 0; i < ARRAY_SIZE(insn->tests); ++i )
    {
        const struct single *s = &insn->tests[i];

        expect(s->type,
               fault ? s->fault : s->trap,
               vector, error);

        user ? exec_user_void(s->fn) : s->fn();

        check();

        /* Avoid 'force' and 'forcered' stubs if FEP isn't available. */
        if ( i == 1 && !xtf_has_fep )
            break;
    }
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

#define TRAP(V)          EXINFO_SYM(V, 0)
#define FAULT(V)         EXINFO_SYM(V, 0)
#define FAULT_EC(V, ...) EXINFO_SYM(V, EXC_EC_SYM(__VA_ARGS__))

/** Tests run in user mode. */
void cpl3_tests(void)
{
    printk("Test cpl3: all perms ok\n");
    {
        test_insn(USER, &int3,    TRAP(BP));
        test_insn(USER, &int_0x3, TRAP(BP));
        test_insn(USER, &icebp,   TRAP(DB));
        test_insn(USER, &int_0x1, TRAP(DB));
        test_insn(USER, &into,    COND(TRAP(OF), FAULT(UD)));
    }

    printk("Test cpl3: p=0\n");
    {
        set_idt_entries_present(false);

        test_insn(USER, &int3,    FAULT_EC(NP, BP));
        test_insn(USER, &int_0x3, FAULT_EC(NP, BP));
        test_insn(USER, &icebp,   FAULT_EC(NP, DB, EXT));
        test_insn(USER, &int_0x1, FAULT_EC(NP, DB));
        test_insn(USER, &into,    COND(FAULT_EC(NP, OF), FAULT(UD)));

        set_idt_entries_present(true);
    }

    printk("Test cpl3: dpl=0\n");
    {
        set_idt_entries_dpl(0);

        test_insn(USER, &int3,    FAULT_EC(GP, BP));
        test_insn(USER, &int_0x3, FAULT_EC(GP, BP));
        test_insn(USER, &icebp,   TRAP(DB)); /* ICEBP doesn't check DPL. */
        test_insn(USER, &int_0x1, FAULT_EC(GP, DB));
        test_insn(USER, &into,    COND(FAULT_EC(GP, OF), FAULT(UD)));

        set_idt_entries_dpl(3);
    }
}

/** Tests run in supervisor mode. */
void cpl0_tests(void)
{
    printk("Test cpl0: all perms ok\n");
    {
        test_insn(KERN, &int3,    TRAP(BP));
        test_insn(KERN, &int_0x3, TRAP(BP));
        test_insn(KERN, &icebp,   TRAP(DB));
        test_insn(KERN, &int_0x1, TRAP(DB));
        test_insn(KERN, &into,    COND(TRAP(OF), FAULT(UD)));
    }

    printk("Test cpl0: p=0\n");
    {
        set_idt_entries_present(false);

        test_insn(KERN, &int3,    FAULT_EC(NP, BP));
        test_insn(KERN, &int_0x3, FAULT_EC(NP, BP));
        test_insn(KERN, &icebp,   FAULT_EC(NP, DB, EXT));
        test_insn(KERN, &int_0x1, FAULT_EC(NP, DB));
        test_insn(KERN, &into,    COND(FAULT_EC(NP, OF), FAULT(UD)));

        set_idt_entries_present(true);
    }
}

void test_main(void)
{
    /*
     * Even if FEP is unavailable, run the tests against real hardware to
     * check the algorithm, but don't claim overall success.
     */
    if ( !xtf_has_fep )
        xtf_skip("FEP support not detected - some tests will be skipped\n");

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
