/**
 * @file tests/umip/main.c
 * @ref test-umip
 *
 * @page test-umip User-Mode Instruction Prevention
 *
 * User-Mode Instruction Prevention (UMIP) is a feature present in new Intel
 * Processors.  When active, it causes the `SGDT`, `SIDT`, `SLDT`, `STR` and
 * `SMSW` instructions to yield @#GP when executed with CPL > 0.  This
 * prevents userspace applications from obtaining sensitive operating system
 * information.
 *
 * @see tests/umip/main.c
 */
#include <xtf.h>

const char test_title[] = "User-Mode Instruction Prevention Tests";

static unsigned long stub_sgdt(unsigned long force)
{
    unsigned long fault = 0;
    desc_ptr tmp;

    asm volatile("testb $1, %b[fep];"
                 "jz 1f;"
                 _ASM_XEN_FEP
                 "1: sgdt %[tmp]; 2:"
                 _ASM_EXTABLE_HANDLER(1b, 2b, %P[rec])
                 : "+a" (fault), [tmp] "=m" (tmp)
                 : [fep] "rm" (force),
                   [rec] "p" (ex_record_fault_eax));

    return fault;
}

static unsigned long stub_sidt(unsigned long force)
{
    unsigned long fault = 0;
    desc_ptr tmp;

    asm volatile("testb $1, %b[fep];"
                 "jz 1f;"
                 _ASM_XEN_FEP
                 "1: sidt %[tmp]; 2:"
                 _ASM_EXTABLE_HANDLER(1b, 2b, %P[rec])
                 : "+a" (fault), [tmp] "=m" (tmp)
                 : [fep] "rm" (force),
                   [rec] "p" (ex_record_fault_eax));

    return fault;
}

static unsigned long stub_sldt(unsigned long force)
{
    unsigned long fault = 0;
    unsigned int tmp;

    asm volatile("testb $1, %b[fep];"
                 "jz 1f;"
                 _ASM_XEN_FEP
                 "1: sldt %[tmp]; 2:"
                 _ASM_EXTABLE_HANDLER(1b, 2b, %P[rec])
                 : "+a" (fault), [tmp] "=r" (tmp)
                 : [fep] "rm" (force),
                   [rec] "p" (ex_record_fault_eax));

    return fault;
}

static unsigned long stub_str(unsigned long force)
{
    unsigned long fault = 0;
    unsigned int tmp;

    asm volatile("testb $1, %b[fep];"
                 "jz 1f;"
                 _ASM_XEN_FEP
                 "1: str %[tmp]; 2:"
                 _ASM_EXTABLE_HANDLER(1b, 2b, %P[rec])
                 : "+a" (fault), [tmp] "=r" (tmp)
                 : [fep] "rm" (force),
                   [rec] "p" (ex_record_fault_eax));

    return fault;
}

static unsigned long stub_smsw(unsigned long force)
{
    unsigned long fault = 0;
    unsigned int tmp;

    asm volatile("testb $1, %b[fep];"
                 "jz 1f;"
                 _ASM_XEN_FEP
                 "1: smsw %[tmp]; 2:"
                 _ASM_EXTABLE_HANDLER(1b, 2b, %P[rec])
                 : "+a" (fault), [tmp] "=r" (tmp)
                 : [fep] "rm" (force),
                   [rec] "p" (ex_record_fault_eax));

    return fault;
}

static unsigned long __user_text stub_user_sgdt(unsigned long force)
{
    unsigned long fault = 0;
    desc_ptr tmp;

    asm volatile("testb $1, %b[fep];"
                 "jz 1f;"
                 _ASM_XEN_FEP
                 "1: sgdt %[tmp]; 2:"
                 _ASM_EXTABLE_HANDLER(1b, 2b, %P[rec])
                 : "+a" (fault), [tmp] "=m" (tmp)
                 : [fep] "rm" (force),
                   [rec] "p" (ex_record_fault_eax));

    return fault;
}

static unsigned long __user_text stub_user_sidt(unsigned long force)
{
    unsigned long fault = 0;
    desc_ptr tmp;

    asm volatile("testb $1, %b[fep];"
                 "jz 1f;"
                 _ASM_XEN_FEP
                 "1: sidt %[tmp]; 2:"
                 _ASM_EXTABLE_HANDLER(1b, 2b, %P[rec])
                 : "+a" (fault), [tmp] "=m" (tmp)
                 : [fep] "rm" (force),
                   [rec] "p" (ex_record_fault_eax));

    return fault;
}

static unsigned long __user_text stub_user_sldt(unsigned long force)
{
    unsigned long fault = 0;
    unsigned int tmp;

    asm volatile("testb $1, %b[fep];"
                 "jz 1f;"
                 _ASM_XEN_FEP
                 "1: sldt %[tmp]; 2:"
                 _ASM_EXTABLE_HANDLER(1b, 2b, %P[rec])
                 : "+a" (fault), [tmp] "=r" (tmp)
                 : [fep] "rm" (force),
                   [rec] "p" (ex_record_fault_eax));

    return fault;
}

static unsigned long __user_text stub_user_str(unsigned long force)
{
    unsigned long fault = 0;
    unsigned int tmp;

    asm volatile("testb $1, %b[fep];"
                 "jz 1f;"
                 _ASM_XEN_FEP
                 "1: str %[tmp]; 2:"
                 _ASM_EXTABLE_HANDLER(1b, 2b, %P[rec])
                 : "+a" (fault), [tmp] "=r" (tmp)
                 : [fep] "rm" (force),
                   [rec] "p" (ex_record_fault_eax));

    return fault;
}

static unsigned long __user_text stub_user_smsw(unsigned long force)
{
    unsigned long fault = 0;
    unsigned int tmp;

    asm volatile("testb $1, %b[fep];"
                 "jz 1f;"
                 _ASM_XEN_FEP
                 "1: smsw %[tmp]; 2:"
                 _ASM_EXTABLE_HANDLER(1b, 2b, %P[rec])
                 : "+a" (fault), [tmp] "=r" (tmp)
                 : [fep] "rm" (force),
                   [rec] "p" (ex_record_fault_eax));

    return fault;
}

static const struct stub {
    unsigned long (*fn)(unsigned long);
    unsigned long (*user_fn)(unsigned long);
    const char *name;
} stubs[] = {
    { stub_sgdt, stub_user_sgdt, "SGDT" },
    { stub_sidt, stub_user_sidt, "SIDT" },
    { stub_sldt, stub_user_sldt, "SLDT" },
    { stub_str,  stub_user_str,  "STR"  },
    { stub_smsw, stub_user_smsw, "SMSW" },
};

static void test_insns(bool umip_active, bool force)
{
    unsigned int i;
    bool user;

    for ( user = false; ; user = true )
    {
        exinfo_t exp = user && umip_active ? EXINFO_SYM(GP, 0) : 0;

        for ( i = 0; i < ARRAY_SIZE(stubs); i++)
        {
            const struct stub *s = &stubs[i];
            exinfo_t ret;

            ret = user ? exec_user_param(s->user_fn, force) : s->fn(force);

            /*
             * Tolerate the instruction emulator not understanding these
             * instructions in older releases of Xen.
             */
            if ( force && ret == EXINFO_SYM(UD, 0) )
            {
                static bool once;

                if ( !once )
                {
                    xtf_skip("Skip: Emulator doesn't implement %s\n", s->name);
                    once = true;
                }

                continue;
            }

            if ( ret != exp )
                xtf_failure("Fail: %s %s (UMIP%c, FEP%c)\n"
                            "  expected %pe\n"
                            "       got %pe\n",
                            user ? "user" : "supervisor", s->name,
                            umip_active ? '+' : '-',
                            force ? '+' : '-',
                            _p(exp), _p(ret));
        }

        if ( user )
            break;
    }
}

static void test_umip(bool umip_active)
{
    test_insns(umip_active, false);

    if ( xtf_has_fep )
        test_insns(umip_active, true);
}

void test_main(void)
{
    if ( !xtf_has_fep )
        xtf_skip("FEP support not detected - some tests will be skipped\n");

    test_umip(false);

    if ( !cpu_has_umip )
    {
        xtf_skip("UMIP is not supported, skip the rest of test\n");

        if ( !write_cr4_safe(read_cr4() | X86_CR4_UMIP) )
            xtf_failure("UMIP unsupported, but setting CR4 bit succeeded\n");

        return;
    }

    /* activate UMIP */
    if ( write_cr4_safe(read_cr4() | X86_CR4_UMIP) )
        return xtf_failure("Fail: Unable to activate UMIP\n");

    test_umip(true);

    /* deactivate UMIP */
    if ( write_cr4_safe(read_cr4() & ~X86_CR4_UMIP) )
        return xtf_failure("Fail: Unable to deactivate UMIP\n");

    test_umip(false);

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
