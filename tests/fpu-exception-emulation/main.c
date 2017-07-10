/**
 * @file tests/fpu-exception-emulation/main.c
 * @ref test-fpu-exception-emulation - Emulation of FPU exceptions
 *
 * @page test-fpu-exception-emulation FPU Exception Emulation
 *
 * Various control register settings have a uniform effect on whether FPU
 * instructions raise an exception or execute normally.
 *
 * Each of the following %cr0 bits are tested:
 * - TS - Task Switched
 * - MP - Monitor Coprocessor
 * - EM - Emulate
 *
 * against instructions from the following sets:
 * - x87
 * - x87 `wait`
 * - MMX
 * - SSE
 * - AVX
 *
 * checking that appropriate exceptions are raised (@#NM or @#UD), or that no
 * exception is raised.
 *
 * Each test is run against real hardware, and forced through the x86
 * instruction emulator (if FEP is available).
 *
 * This test covers XSA-190, where @#NM was not being raised appropriately,
 * therefore interfering with lazy FPU task switching in the guest.
 *
 * @todo Extend to include unmasked pending exceptions.  There is definitely
 * work required in the instruction emulator to support this properly.
 *
 * @see tests/fpu-exception-emulation/main.c
 */
#include <xtf.h>

#include <arch/decode.h>
#include <arch/exinfo.h>
#include <arch/processor.h>
#include <arch/symbolic-const.h>

const char test_title[] = "FPU Exception Emulation";

#define CR0_SYM(...) TOK_OR(X86_CR0_, ##__VA_ARGS__)
#define CR0_MASK CR0_SYM(EM, MP, TS)

struct test_cfg
{
    unsigned long cr0;
    exinfo_t fault;
};

static unsigned long default_cr0;

/**
 * x87 coprocessor.  Should raise @#NM if emulation is enabled (no coprocessor
 * available) or if the task has switched.
 */
static const struct test_cfg x87[] =
{
    { CR0_SYM(          ), 0 },
    { CR0_SYM(        TS), EXINFO_SYM(NM, 0) },
    { CR0_SYM(    MP    ), 0 },
    { CR0_SYM(    MP, TS), EXINFO_SYM(NM, 0) },
    { CR0_SYM(EM        ), EXINFO_SYM(NM, 0) },
    { CR0_SYM(EM,     TS), EXINFO_SYM(NM, 0) },
    { CR0_SYM(EM, MP    ), EXINFO_SYM(NM, 0) },
    { CR0_SYM(EM, MP, TS), EXINFO_SYM(NM, 0) },
};

exinfo_t probe_x87(bool force)
{
    exinfo_t fault = 0;

    asm volatile ("test %[fep], %[fep];"
                  "jz 1f;"
                  _ASM_XEN_FEP
                  "1: fnop; 2:"
                  _ASM_EXTABLE_HANDLER(1b, 2b, ex_record_fault_eax)
                  : "+a" (fault)
                  : [fep] "q" (force),
                    "X" (ex_record_fault_eax));

    return fault;
}

/**
 * The x87 `wait` instruction.  This instruction behaves differently, and
 * should only raise @#NM when monitoring the coprocessor and the task has
 * switched.
 */
static const struct test_cfg x87_wait[] =
{
    { CR0_SYM(          ), 0 },
    { CR0_SYM(        TS), 0 },
    { CR0_SYM(    MP    ), 0 },
    { CR0_SYM(    MP, TS), EXINFO_SYM(NM, 0) },
    { CR0_SYM(EM        ), 0 },
    { CR0_SYM(EM,     TS), 0 },
    { CR0_SYM(EM, MP    ), 0 },
    { CR0_SYM(EM, MP, TS), EXINFO_SYM(NM, 0) },
};

exinfo_t probe_x87_wait(bool force)
{
    exinfo_t fault = 0;

    asm volatile ("test %[fep], %[fep];"
                  "jz 1f;"
                  _ASM_XEN_FEP
                  "1: wait; 2:"
                  _ASM_EXTABLE_HANDLER(1b, 2b, ex_record_fault_eax)
                  : "+a" (fault)
                  : [fep] "q" (force),
                    "X" (ex_record_fault_eax));

    return fault;
}

/**
 * MMX and SSE instructions.  Emulation is unsupported (thus raising @#UD),
 * but @#NM should be raised if the task has been switched.
 */
static const struct test_cfg mmx_sse[] =
{
    { CR0_SYM(          ), 0 },
    { CR0_SYM(        TS), EXINFO_SYM(NM, 0) },
    { CR0_SYM(    MP    ), 0 },
    { CR0_SYM(    MP, TS), EXINFO_SYM(NM, 0) },
    { CR0_SYM(EM        ), EXINFO_SYM(UD, 0) },
    { CR0_SYM(EM,     TS), EXINFO_SYM(UD, 0) },
    { CR0_SYM(EM, MP    ), EXINFO_SYM(UD, 0) },
    { CR0_SYM(EM, MP, TS), EXINFO_SYM(UD, 0) },
};

exinfo_t probe_mmx(bool force)
{
    exinfo_t fault = 0;

    asm volatile ("test %[fep], %[fep];"
                  "jz 1f;"
                  _ASM_XEN_FEP
                  "1: movq %%mm0, %%mm0; 2:"
                  _ASM_EXTABLE_HANDLER(1b, 2b, ex_record_fault_eax)
                  : "+a" (fault)
                  : [fep] "q" (force),
                    "X" (ex_record_fault_eax));

    return fault;
}

exinfo_t probe_sse(bool force)
{
    exinfo_t fault = 0;

    asm volatile ("test %[fep], %[fep];"
                  "jz 1f;"
                  _ASM_XEN_FEP
                  "1: movups %%xmm0, %%xmm0; 2:"
                  _ASM_EXTABLE_HANDLER(1b, 2b, ex_record_fault_eax)
                  : "+a" (fault)
                  : [fep] "q" (force),
                    "X" (ex_record_fault_eax));

    return fault;
}

/**
 * AVX instructions.  The emulation flag is meaningless,
 * but @#NM should be raised if the task has been switched.
 */
static const struct test_cfg avx[] =
{
    { CR0_SYM(          ), 0 },
    { CR0_SYM(        TS), EXINFO_SYM(NM, 0) },
    { CR0_SYM(    MP    ), 0 },
    { CR0_SYM(    MP, TS), EXINFO_SYM(NM, 0) },
    { CR0_SYM(EM        ), 0 },
    { CR0_SYM(EM,     TS), EXINFO_SYM(NM, 0) },
    { CR0_SYM(EM, MP    ), 0 },
    { CR0_SYM(EM, MP, TS), EXINFO_SYM(NM, 0) },
};

static exinfo_t probe_avx(bool force)
{
    exinfo_t fault = 0;

    asm volatile ("test %[fep], %[fep];"
                  "jz 1f;"
                  _ASM_XEN_FEP
                  "1: vmovups %%xmm0, %%xmm0; 2:"
                  _ASM_EXTABLE_HANDLER(1b, 2b, ex_record_fault_eax)
                  : "+a" (fault)
                  : [fep] "q" (force),
                    "X" (ex_record_fault_eax));

    return fault;
}

void run_sequence(const struct test_cfg *seq, unsigned int nr,
                  unsigned int (*fn)(bool), bool force, exinfo_t override)
{
    unsigned int i;

    for ( i = 0; i < nr; ++i )
    {
        const struct test_cfg *t = &seq[i];
        exinfo_t res, exp = override ?: t->fault;

        write_cr0((default_cr0 & ~CR0_MASK) | t->cr0);
        res = fn(force);

        if ( res != exp )
        {
            char cr0str[12];

            snprintf(cr0str, sizeof(cr0str), "%s%s%s",
                     t->cr0 & X86_CR0_EM ? " EM" : "",
                     t->cr0 & X86_CR0_MP ? " MP" : "",
                     t->cr0 & X86_CR0_TS ? " TS" : "");

            xtf_failure("  Expected %pe, got %pe (cr0:%s)\n",
                        _p(exp), _p(res), cr0str[0] ? cr0str : " - ");
        }
    }
}

void run_tests(bool force)
{
    if ( cpu_has_fpu )
    {
        printk("Testing%s x87\n", force ? " emulated" : "");
        run_sequence(x87, ARRAY_SIZE(x87), probe_x87, force, 0);

        printk("Testing%s x87 wait\n", force ? " emulated" : "");
        run_sequence(x87_wait, ARRAY_SIZE(x87_wait), probe_x87_wait, force, 0);
    }

    if ( cpu_has_mmx )
    {
        printk("Testing%s MMX\n", force ? " emulated" : "");
        run_sequence(mmx_sse, ARRAY_SIZE(mmx_sse), probe_mmx, force, 0);
    }

    if ( cpu_has_sse )
    {
        unsigned long cr4 = read_cr4();

        printk("Testing%s SSE\n", force ? " emulated" : "");
        write_cr4(cr4 & ~X86_CR4_OSFXSR);
        run_sequence(mmx_sse, ARRAY_SIZE(mmx_sse), probe_sse, force,
                     EXINFO_SYM(UD, 0));

        printk("Testing%s SSE (CR4.OSFXSR)\n", force ? " emulated" : "");
        write_cr4(cr4 | X86_CR4_OSFXSR);
        run_sequence(mmx_sse, ARRAY_SIZE(mmx_sse), probe_sse, force, 0);

        write_cr4(cr4);
    }

    if ( cpu_has_avx )
    {
        unsigned long cr4 = read_cr4();
        unsigned long xcr0;

        printk("Testing%s AVX\n", force ? " emulated" : "");
        write_cr4(cr4 & ~X86_CR4_OSXSAVE);
        run_sequence(avx, ARRAY_SIZE(avx), probe_avx, force,
                     EXINFO_SYM(UD, 0));

        printk("Testing%s AVX (CR4.OSXSAVE)\n", force ? " emulated" : "");
        write_cr4(cr4 | X86_CR4_OSXSAVE);
        xcr0 = read_xcr0();
        write_xcr0(xcr0 & ~XSTATE_YMM);
        run_sequence(avx, ARRAY_SIZE(avx), probe_avx, force,
                     EXINFO_SYM(UD, 0));

        printk("Testing%s AVX (CR4.OSXSAVE+XCR0.YMM)\n", force ? " emulated" : "");
        write_xcr0(xcr0 | XSTATE_SSE | XSTATE_YMM);
        run_sequence(avx, ARRAY_SIZE(avx), probe_avx, force, 0);

        write_xcr0(xcr0);
        write_cr4(cr4);
    }
}

void test_main(void)
{
    default_cr0 = read_cr0();

    run_tests(false);

    if ( !xtf_has_fep )
        xtf_skip("FEP support not detected - some tests will be skipped\n");
    else
        run_tests(true);

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
