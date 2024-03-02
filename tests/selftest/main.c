/**
 * @file tests/selftest/main.c
 * @ref test-selftest - Sanity tests for the XTF environment.
 *
 * @page test-selftest Selftest
 *
 * Sanity tests for the framework environment and functionality.  Failure of
 * these tests tend to suggest bugs with the framework itself.
 *
 * @see tests/selftest/main.c
 */
#include <xtf.h>

const char test_title[] = "XTF Selftests";
bool has_xenstore = true;

static void test_xenstore(void)
{
    printk("Test: Xenstore read\n");

    const char *domid_str = xenstore_read("domid");

    if ( !domid_str )
        return xtf_failure("Fail: No domid value returned\n");

    if ( domid_str[0] == '\0' )
        return xtf_failure("Fail: domid value empty\n");

    unsigned int i;
    for ( i = 0; domid_str[i]; ++i )
    {
        if ( domid_str[i] < '0' || domid_str[i] > '9' )
            return xtf_failure("Fail: unexpected domid value '%s'\n",
                               domid_str);
    }

    printk("  Found domid %s\n", domid_str);
}

static void test_extable(void)
{
    printk("Test: Exception Table\n");

    /*
     * Check that control flow is successfully redirected with a ud2a
     * instruction and appropriate extable entry.
     */
    asm volatile ("1: ud2a; 2:"
                  _ASM_EXTABLE(1b, 2b));
}

enum {
    USER_not_seen,
    USER_seen,
    USER_bad_cs,
};

static unsigned long __user_text test_exec_user_cpl3(void)
{
    return ((read_cs() & 3) == 3) ? USER_seen : USER_bad_cs;
}

static void test_exec_user(void)
{
    unsigned int res;

    printk("Test: Userspace execution\n");

    res = exec_user(test_exec_user_cpl3);

    switch ( res )
    {
    case USER_seen:
        /* Success */
        break;

    case USER_bad_cs:
        xtf_failure("Fail: Not at cpl3\n");
        break;

    default:
        xtf_failure("Fail: Did not execute function\n");
        break;
    }
}

static void test_NULL_unmapped(void)
{
    unsigned int tmp;
    exinfo_t got = 0;

    printk("Test: NULL unmapped\n");

    asm volatile ("1: mov 0, %[tmp]; 2:"
                  _ASM_EXTABLE_HANDLER(1b, 2b, %P[rec])
                  : "+a" (got),
                    [tmp] "=r" (tmp)
                  : [rec] "p" (ex_record_fault_eax));

    if ( got != EXINFO_SYM(PF, 0) )
        xtf_failure("Fail: Expected #PF, got %pe\n", _p(got));
}

bool do_unhandled_exception(struct cpu_regs *regs)
{
    extern unsigned long hook_fault[], hook_fixup[];

    if ( _p(regs->ip) != hook_fault )
    {
        xtf_failure("Fail: Expected fault at %p, got %p\n",
                    hook_fault, _p(regs->ip));
        return false;
    }

    regs->ip = _u(hook_fixup);
    return true;
}

static void test_unhandled_exception_hook(void)
{
    printk("Test: Unhandled Exception Hook\n");

    /* Check that the hook catches the exception, and fix it up. */
    asm volatile ("hook_fault: ud2a; hook_fixup:");
}

static bool test_extable_handler_handler_run;
static bool test_extable_handler_handler(struct cpu_regs *regs,
                                         const struct extable_entry *ex)
{
    test_extable_handler_handler_run = true;
    regs->ip = ex->fixup;
    return true;
}

static void test_extable_handler(void)
{
    printk("Test: Exception Table Handler\n");

    asm volatile ("1: ud2a; 2:"
                  _ASM_EXTABLE_HANDLER(1b, 2b, %P[hnd])
                  :: [hnd] "p" (test_extable_handler_handler));

    if ( !test_extable_handler_handler_run )
        xtf_failure("Fail: Custom handler didn't run\n");
}

void test_idte_handler(void);
asm ("test_idte_handler:;"
#if defined (CONFIG_PV) && defined (CONFIG_64BIT)
     "pop %rcx; pop %r11;"
#endif
     "mov $0x1e51c0de, %eax;"
#if defined (CONFIG_HVM)
     __ASM_SEL(iretl, iretq) ";"
#else /* CONFIG_HVM */
#ifdef __x86_64__
     "push $0;"
#endif
     "jmp HYPERCALL_iret;"
#endif
    );

static const struct xtf_idte idte = {
    .addr = _u(test_idte_handler),
    /* PV guests need DPL1, HVM need DPL0. */
    .dpl = IS_DEFINED(CONFIG_PV) ? 1 : 0,
    .cs = __KERN_CS,
};

static void test_custom_idte(void)
{
    printk("Test: Custom IDT entry\n");

    int rc = xtf_set_idte(X86_VEC_AVAIL, &idte);

    if ( rc )
        return xtf_failure("Fail: xtf_set_idte() returned %d\n", rc);

    unsigned int res;
    asm volatile ("int $%c[vec]"
                  : "=a" (res)
                  : "0" (0),
                    [vec] "i" (X86_VEC_AVAIL));

    if ( res != 0x1e51c0de )
        xtf_failure("Fail: Unexpected result %#x\n", res);
};

static void test_driver_init(void)
{
    int rc;

    printk("Test: Driver basic initialisation\n");

    if ( IS_DEFINED(CONFIG_HVM) )
    {
        rc = apic_init(APIC_MODE_XAPIC);

        /* Cope with guests which have LAPIC emulation disabled. */
        if ( rc != -ENODEV )
        {
            if ( rc )
                xtf_failure("Fail: apic_init(XAPIC) returned %d\n", rc);

            if ( cpu_has_x2apic )
            {
                rc = apic_init(APIC_MODE_X2APIC);

                if ( rc )
                    xtf_failure("Fail: apic_init(X2APIC) returned %d\n", rc);
            }
        }

        rc = hpet_init();
        if ( rc && rc != -ENODEV )
            xtf_failure("Fail: hpet_init() returned %d\n", rc);

        rc = ioapic_init();
        if ( rc && rc != -ENODEV )
            xtf_failure("Fail: ioapic_init() returned %d\n", rc);
    }

    rc = xenstore_init();
    has_xenstore = !rc;
    if ( rc && rc != -ENODEV )
        xtf_failure("Fail: xenstore_init() returned %d\n", rc);

    rc = xtf_init_grant_table(1);
    if ( rc )
        xtf_failure("Fail: xtf_init_grant_table(1) returned %d\n", rc);

    rc = xtf_init_grant_table(2);
    if ( rc && rc != -ENODEV )
        xtf_failure("Fail: xtf_init_grant_table(2) returned %d\n", rc);
}

static void test_vsnprintf_crlf_one(const char *fmt, ...)
{
    va_list args;

    char buf[4];
    int rc;

    va_start(args, fmt);
    rc = vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);

    if ( rc != 1 )
        return xtf_failure("Fail: '%s', expected length 1, got %d\n", fmt, rc);
    if ( strcmp(buf, "\n") )
        return xtf_failure("Fail: '%s', expected \"\\n\", got %*ph\n",
                           fmt, (int)sizeof(buf), buf);

    va_start(args, fmt);
    rc = vsnprintf_internal(buf, sizeof(buf), fmt, args, LF_TO_CRLF);
    va_end(args);

    if ( rc != 2 )
        return xtf_failure("Fail: '%s', expected length 2, got %d\n", fmt, rc);
    if ( strcmp(buf, "\r\n") )
        return xtf_failure("Fail: '%s', expected \"\\r\\n\", got %*ph\n",
                           fmt, (int)sizeof(buf), buf);
}

static void test_vsnprintf_crlf(void)
{
    printk("Test: vsnprintf() with CRLF expansion\n");

    test_vsnprintf_crlf_one("\n");
    test_vsnprintf_crlf_one("%c", '\n');
    test_vsnprintf_crlf_one("%s", "\n");
}

void test_main(void)
{
    /*
     * Wherever possible, enable SMEP and SMAP to test the safety of the
     * exec_user infrastructure.
     */
    if ( IS_DEFINED(CONFIG_HVM) )
    {
        unsigned long cr4 = read_cr4(), ocr4 = cr4;

        if ( cpu_has_smep )
            cr4 |= X86_CR4_SMEP;
        if ( cpu_has_smap )
            cr4 |= X86_CR4_SMAP;

        if ( cr4 != ocr4 )
            write_cr4(cr4);
    }

    test_extable();
    test_exec_user();
    if ( CONFIG_PAGING_LEVELS > 0 )
        test_NULL_unmapped();
    test_unhandled_exception_hook();
    test_extable_handler();
    test_custom_idte();
    test_driver_init();
    test_vsnprintf_crlf();

    if ( has_xenstore )
        test_xenstore();

    xtf_success(NULL);
}

/*
 * Inline assembly checks.
 *
 * Needs to be written out into an object file to cause build failures.
 * Nothing executes the resulting code.
 *
 * - push/pop %reg need to use unsigned long types to avoid trying to allocate
 *   32bit registers, which aren't encodable in 64bit.
 * - push $imm can't encode 64bit integers (only 32bit sign extended)
 */
static void __used asm_checks(void)
{
    read_flags();

#ifdef __x86_64__
    unsigned long tmp = 0xdead0000c0deULL;

    write_flags(tmp);
    write_cs(tmp);
#endif
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
