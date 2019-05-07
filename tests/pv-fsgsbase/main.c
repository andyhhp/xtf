/**
 * @file tests/pv-fsgsbase/main.c
 * @ref test-pv-fsgsbase
 *
 * @page test-pv-fsgsbase PV FSGSBASE behaviour
 *
 * Tests for the behaviour of FSGSBASE handling in PV guests.
 *
 * Before XSA-293, Xen had a bug whereby a PV guest could set and clear
 * FSGSBASE in its view of %%cr4, but Xen left the feature actually enabled in
 * hardware.
 *
 * This in practice leaves the `{RD,WR}{FS,GS}BASE` instructions usable behind
 * the back of the guest kernel, and can cause state corruption on task switch
 * if the guest kernel is expecting these instructions to be unusable.
 *
 * Another awkward point is that at the time of writing, Xen doesn't raise
 * @#GP faults for bad %%cr4 updates, opting instead to discard bits it
 * doesn't like.
 *
 *
 * This test tries to cross-reference the architectural behaviour between the
 * FSGSBASE CPUID bit, the enable bit in %%cr4, and whether the instructions
 * are actually usable.
 *
 * In addition, it checks that @#GP faults are raised appropriately for trying
 * to write non-canonical addresses.  This is anticipated to be a problem when
 * Xen supports LA57 mode.
 *
 * @see tests/pv-fsgsbase/main.c
 */
#include <xtf.h>

const char test_title[] = "Test PV FSGSBASE behaviour";

static exinfo_t stub_rdfsbase(unsigned long unused)
{
    unsigned long tmp;
    exinfo_t fault = 0;

    asm volatile ("1: rdfsbase %[val]; 2:"
                  _ASM_EXTABLE_HANDLER(1b, 2b, %P[rec])
                  : "+a" (fault), [val] "=r" (tmp)
                  : [rec] "p" (ex_record_fault_eax));

    return fault;
}

static exinfo_t stub_rdgsbase(unsigned long unused)
{
    unsigned long tmp;
    exinfo_t fault = 0;

    asm volatile ("1: rdgsbase %[val]; 2:"
                  _ASM_EXTABLE_HANDLER(1b, 2b, %P[rec])
                  : "+a" (fault), [val] "=r" (tmp)
                  : [rec] "p" (ex_record_fault_eax));

    return fault;
}

static exinfo_t stub_wrfsbase(unsigned long val)
{
    exinfo_t fault = 0;

    asm volatile ("1: wrfsbase %[val]; 2:"
                  _ASM_EXTABLE_HANDLER(1b, 2b, %P[rec])
                  : "+a" (fault)
                  : [val] "r" (val), [rec] "p" (ex_record_fault_eax));

    return fault;
}

static exinfo_t stub_wrgsbase(unsigned long val)
{
    exinfo_t fault = 0;

    asm volatile ("1: wrgsbase %[val]; 2:"
                  _ASM_EXTABLE_HANDLER(1b, 2b, %P[rec])
                  : "+a" (fault)
                  : [val] "r" (val), [rec] "p" (ex_record_fault_eax));

    return fault;
}

static void test_fsgsbase_insns(bool should_ud)
{
    static const struct test {
        const char *name;
        exinfo_t (*fn)(unsigned long);
    } tests[] = {
        { "rdfsbase", stub_rdfsbase },
        { "rdgsbase", stub_rdgsbase },
        { "wrfsbase", stub_wrfsbase },
        { "wrgsbase", stub_wrgsbase },
    };
    unsigned int i;
    exinfo_t exp = should_ud ? EXINFO_SYM(UD, 0) : 0;

    for ( i = 0; i < ARRAY_SIZE(tests); ++i )
    {
        const struct test *t = &tests[i];
        exinfo_t res = t->fn(0);

        if ( res != exp )
            xtf_failure("Fail: Testing '%s'\n"
                        "  expected %pe, got %pe\n",
                        t->name, _p(exp), _p(res));
    }
}

static void test_wrfsbase_values(void)
{
    static const struct test {
        unsigned long val;
        exinfo_t fault;
    } tests[] = {
        { 0x0000000000000000ul, 0 },
        { 0x00007ffffffffffful, 0 },
        { 0x0000800000000000ul, EXINFO_SYM(GP, 0) },
        { 0x7ffffffffffffffful, EXINFO_SYM(GP, 0) },
        { 0x8000000000000000ul, EXINFO_SYM(GP, 0) },
        { 0xffff7ffffffffffful, EXINFO_SYM(GP, 0) },
        { 0xffff800000000000ul, 0 },
        { 0xfffffffffffffffful, 0 },
    };
    unsigned int i;

    for ( i = 0; i < ARRAY_SIZE(tests); ++i )
    {
        const struct test *t = &tests[i];
        exinfo_t res = stub_wrfsbase(t->val);

        if ( res != t->fault )
            xtf_failure("Fail: Testing WRFSBASE %016lx \n"
                        "  expected %pe, got %pe\n",
                        t->val , _p(t->fault), _p(res));
    }
}

/*
 * Bad updates to CR4 should fault, but at the time of writing, Xen squashes
 * all faults and breaks the use of the *_safe() functions, and drops the
 * feature.
 *
 * Wrap write_cr4_safe() with a variant which reads cr4 back, to see if write
 * fully took effect.
 */
static bool pv_write_cr4_safe(unsigned long val)
{
    unsigned long old = read_cr4(), changed = old ^ val;
    bool fault = write_cr4_safe(val);

    if ( !fault )
    {
        unsigned long new = read_cr4();

        fault = (old ^ new) != changed;
    }

    return fault;
}

void test_main(void)
{
    unsigned long cr4;

    if ( !cpu_has_fsgsbase )
        xtf_skip("FSGSBASE support not detected\n");

    /* Check that CR4.FSGSBASE is clear to begin with. */
    cr4 = read_cr4();
    if ( cr4 & X86_CR4_FSGSBASE )
    {
        xtf_failure("Fail: Initial CR4.FSGSBASE state should be clear\n");

        if ( pv_write_cr4_safe(cr4 &= ~X86_CR4_FSGSBASE) )
            return xtf_failure("Fail: Fault while trying to clear CR4.FSGSBASE\n");
    }

    /*
     * Check that the {RD,WR}{FS,GS}BASE instructions are disabled.  When
     * vulnerable to XSA-293, this check will fail.
     */
    test_fsgsbase_insns(true);

    if ( !cpu_has_fsgsbase )
    {
        /* If the FSGSBASE feature isn't visible, check we can't turn it on. */
        if ( !pv_write_cr4_safe(cr4 |= X86_CR4_FSGSBASE) )
            xtf_failure("Fail: Able to set CR4.FSGSBASE without the feature\n");

        return;
    }

    /* Check we can turn CR4.FSGSBASE on. */
    if ( pv_write_cr4_safe(cr4 |= X86_CR4_FSGSBASE) )
        xtf_failure("Fail: Unable to enable CR4.FSGSBASE\n");

    /* Check that {RD,WR}{FS,GS}BASE instructions are enabled. */
    test_fsgsbase_insns(false);

    /* Check that WRFSBASE faults on appropriate values. */
    test_wrfsbase_values();

    /* Check we can turn CR4.FSGSBASE off again. */
    if ( pv_write_cr4_safe(cr4 &= ~X86_CR4_FSGSBASE) )
        xtf_failure("Fail: Unable to enable CR4.FSGSBASE\n");

    /* Check that {RD,WR}{FS,GS}BASE instructions are disabled again. */
    test_fsgsbase_insns(true);

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
