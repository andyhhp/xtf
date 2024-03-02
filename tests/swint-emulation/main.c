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

#ifdef __i386__
# define COND(_32, _64) _32
#else
# define COND(_32, _64) _64
#endif

#define EXINFO_TRAP EXINFO_AVAIL0

enum mode { KERN, USER };

bool ex_record_trap_eax(struct cpu_regs *regs, const struct extable_entry *ex)
{
    regs->ax = EXINFO(regs->entry_vector, regs->error_code) | EXINFO_TRAP;
    regs->ip = ex->fixup;

    return true;
}

struct insn
{
    const char *name;
    unsigned long (*fn[4])(void);
    unsigned long (*user_fn[4])(void);
};

const struct insn int3 = {
    "int3",
    {
        stub_int3,
        stub_int3_A,
        stub_int3_F,
        stub_int3_FA,
    },
    {
        stub_user_int3,
        stub_user_int3_A,
        stub_user_int3_F,
        stub_user_int3_FA,
    },
};

const struct insn int_0x3 = {
    "int $3",
    {
        stub_int_0x3,
        stub_int_0x3_A,
        stub_int_0x3_F,
        stub_int_0x3_FA,
    },
    {
        stub_user_int_0x3,
        stub_user_int_0x3_A,
        stub_user_int_0x3_F,
        stub_user_int_0x3_FA,
    },
};

const struct insn icebp = {
    "icebp",
    {
        stub_icebp,
        stub_icebp_A,
        stub_icebp_F,
        stub_icebp_FA,
    },
    {
        stub_user_icebp,
        stub_user_icebp_A,
        stub_user_icebp_F,
        stub_user_icebp_FA,
    },
};

const struct insn int_0x1 = {
    "int $1",
    {
        stub_int_0x1,
        stub_int_0x1_A,
        stub_int_0x1_F,
        stub_int_0x1_FA,
    },
    {
        stub_user_int_0x1,
        stub_user_int_0x1_A,
        stub_user_int_0x1_F,
        stub_user_int_0x1_FA,
    },
};

const struct insn into = {
    "into",
    {
        stub_into,
        stub_into_A,
        stub_into_F,
        stub_into_FA,
    },
    {
        stub_user_into,
        stub_user_into_A,
        stub_user_into_F,
        stub_user_into_FA,
    },
};

void test_insn(enum mode user, const struct insn *insn, exinfo_t exp)
{
    printk("  Testing %s\n", insn->name);

    for ( unsigned int i = 0; i < ARRAY_SIZE(insn->fn); ++i )
    {
        exinfo_t got;

        got = user ? exec_user(insn->user_fn[i]) : insn->fn[i]();

        if ( exp != got )
            xtf_failure("    Fail (Force%c, Addr%c): expected %pe %s, got %pe %s\n",
                        i & 1 ? '+' : '-',
                        i & 2 ? '+' : '-',
                        _p(exp), exp & EXINFO_TRAP ? "trap" : "fault",
                        _p(got), got & EXINFO_TRAP ? "trap" : "fault");

        /* Avoid FEP stubs if FEP isn't available. */
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

#define TRAP(V)          EXINFO_SYM(V, 0) | EXINFO_TRAP
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

    cpl0_tests();
    cpl3_tests();

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
