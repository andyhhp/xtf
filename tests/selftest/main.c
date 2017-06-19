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

#include <arch/apic.h>
#include <arch/idt.h>
#include <arch/processor.h>
#include <arch/segment.h>

const char test_title[] = "XTF Selftests";

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

static bool check_nr_entries(unsigned int nr)
{
    unsigned int entries = xtf_exlog_entries();

    if ( entries != nr )
    {
        xtf_failure("Fail: expected %u entries, got %u\n",
                    nr, entries);
        return false;
    }

    return true;
}

static bool check_exlog_entry(unsigned int entry, unsigned int cs,
                              unsigned long ip, unsigned int ev,
                              unsigned int ec)
{
    exlog_entry_t *e = xtf_exlog_entry(entry);

    /* Check whether the log entry is available. */
    if ( !e )
    {
        xtf_failure("Fail: unable to retrieve log entry %u\n", entry);
        return false;
    }

    /* Check whether the log entry is correct. */
    if ( (e->ip != ip) || (e->cs != cs) || (e->ec != ec) || (e->ev != ev) )
    {
        xtf_failure("Fail: exlog entry:\n"
                    "  Expected: %04x:%p, ec %04x, vec %u\n"
                    "       Got: %04x:%p, ec %04x, vec %u\n",
                    cs, _p(ip), ec, ev, e->cs, _p(e->ip), e->ec, e->ev);
        return false;
    }

    return true;
}

static void test_exlog(void)
{
    extern unsigned long exlog_int3[] asm(".Lexlog_int3");
    extern unsigned long exlog_ud2a[] asm(".Lexlog_ud2a");

    printk("Test: Exception Logging\n");

    xtf_exlog_start();

    /* Check that no entries have been logged thus far. */
    if ( !check_nr_entries(0) )
        goto out;

    asm volatile ("int3; .Lexlog_int3:"
                  _ASM_TRAP_OK(.Lexlog_int3));

    /* Check that one entry has now been logged. */
    if ( !check_nr_entries(1) ||
         !check_exlog_entry(0, __KERN_CS, _u(exlog_int3), X86_EXC_BP, 0) )
        goto out;

    asm volatile (".Lexlog_ud2a: ud2a; 1:"
                  _ASM_EXTABLE(.Lexlog_ud2a, 1b));

    /* Check that two entries have now been logged. */
    if ( !check_nr_entries(2) ||
         !check_exlog_entry(1, __KERN_CS, _u(exlog_ud2a), X86_EXC_UD, 0) )
        goto out;

    xtf_exlog_reset();

    /* Check that no entries now exist. */
    if ( !check_nr_entries(0) )
        goto out;

    asm volatile ("int3; 1:"
                  _ASM_TRAP_OK(1b));

    /* Check that one entry now exists. */
    if ( !check_nr_entries(1) )
        goto out;

    xtf_exlog_stop();

    /* Check that one entry still exists. */
    if ( !check_nr_entries(1) )
        goto out;

    asm volatile ("int3; 1:"
                  _ASM_TRAP_OK(1b));

    /* Check that the previous breakpoint wasn't logged. */
    if ( !check_nr_entries(1) )
        goto out;

 out:
    xtf_exlog_reset();
    xtf_exlog_stop();
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
    extern unsigned long label_test_NULL_unmapped[];
    unsigned long tmp;

    printk("Test: NULL unmapped\n");

    xtf_exlog_start();

    asm volatile ("label_test_NULL_unmapped: mov 0, %0; 2:"
                  _ASM_EXTABLE(label_test_NULL_unmapped, 2b)
                  : "=q" (tmp) :: "memory");

    if ( check_nr_entries(1) )
        check_exlog_entry(0, __KERN_CS, _u(label_test_NULL_unmapped), X86_EXC_PF, 0);

    xtf_exlog_stop();
}

static bool local_unhandled_exception_hook(struct cpu_regs *regs)
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
    xtf_unhandled_exception_hook = local_unhandled_exception_hook;

    asm volatile ("hook_fault: ud2a; hook_fixup:" ::: "memory");

    xtf_unhandled_exception_hook = NULL;
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
                  _ASM_EXTABLE_HANDLER(1b, 2b,
                                       test_extable_handler_handler)
                  :: "X" (test_extable_handler_handler));

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
#ifdef __x86_64__
     "rex64 "
#endif
     "iret;"
#else /* CONFIG_HVM */
#ifdef __x86_64__
     "push $0;"
#endif
     "jmp HYPERCALL_iret;"
#endif
    );

static void test_custom_idte(void)
{
    struct xtf_idte idte =
        {
            .addr = _u(test_idte_handler),
            /* PV guests need DPL1, HVM need DPL0. */
            .dpl = IS_DEFINED(CONFIG_PV) ? 1 : 0,
            .cs = __KERN_CS,
        };

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
        if ( rc && rc != -ENODEV )
            xtf_failure("Fail: apic_init() returned %d\n", rc);
    }

    rc = xtf_init_grant_table(1);
    if ( rc )
        xtf_failure("Fail: xtf_init_grant_table(1) returned %d\n", rc);

    rc = xtf_init_grant_table(2);
    if ( rc && rc != -ENODEV )
        xtf_failure("Fail: xtf_init_grant_table(2) returned %d\n", rc);
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

    test_xenstore();
    test_extable();
    test_exlog();
    test_exec_user();
    if ( CONFIG_PAGING_LEVELS > 0 )
        test_NULL_unmapped();
    test_unhandled_exception_hook();
    test_extable_handler();
    test_custom_idte();
    test_driver_init();

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
