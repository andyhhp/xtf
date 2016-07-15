/**
 * @file tests/invlpg/main.c
 * @ref test-invlpg `invlpg` instruction handling
 *
 * @page test-invlpg Invlpg Handling
 *
 * The `invlpg` instruction was introduced in the 486 processor.  Its purpose
 * is to invalidate a specific TLB entry without being a general TLB flush.
 * It takes a single memory operand, but doesn't behave like other
 * instructions referencing memory.
 *
 * It is not documented to suffer the usual memory-based faults (@#GP/@#SS)
 * for a bad segment, or segment limit violation.  It is documented to be a
 * NOP when given a non-canonical address, as opposed to raising a fault.
 *
 * Experimentally, it does take into account segment bases when calculating
 * which mapping to invalidate, but will also invalidate mappings beyond the
 * segment limit.
 *
 * This test is divided into two parts.
 *
 *
 * ### 1) TLB Refill test ###
 *
 * This code was originally built to confirm whether segment bases were taken
 * into account by the `invlpg` instruction.  It is kept because it is useful
 * to compare the behaviour of the HAP and shadow paging modes, as well as the
 * emulated invlpg path in each mode.
 *
 * The algorithm relies on the fact that a hardware pagetable walk which loads
 * a mapping into the TLB for writing will perform an atomic set of the
 * Accessed and Dirty bits in the pagetable entry.  Subsequent writes via a
 * mapping present in the TLB do not alter the pagetable entries.
 *
 * Two mappings are chosen (in this example, the mapping for 4k and 8k
 * physical addresses, but any two will do).  Writes are performed to force a
 * TLB fill, then the A/D bits are cleared in the PTEs.  An `invlpg`
 * instruction is issued to invalidate one of mappings, and further writes are
 * performed.
 *
 * By inspecting the A/D bits in the PTEs after the second write, it can be
 * determined which mappings where invalidated.  A mapping which wasn't
 * invalidated will not require a refill, so the A/D bits will be left clear.
 * A mapping which was invalidated will necessarily require a refill, which
 * will set the A/D bits.
 *
 * There is a race condition with vcpu scheduling; if a reschedule occurs
 * between the two sets of writes, the TLB might be flushed for reasons not
 * related to this test.  As a result, if both mappings are seen to refill,
 * the test is repeated (up to a maximum number of tries).
 *
 * The interaction of segment bases can be identified by issuing an `invlpg`
 * against the first mapping, with a segment base with would shift the linear
 * address to the second mapping, by observing which mapping was refilled into
 * the TLB.
 *
 *
 * ### 2) Faulting conditions ###
 *
 * This part of the test checks that the `invlpg` instruction doesn't fault
 * under conditions which a normal instruction with a memory operand would.
 *
 * - A virtual address with a valid mapping.
 * - A virtual address with an invalid mapping.
 * - A NULL segment override.
 * - Beyond the segment limit.
 * - Between the base and limit of an expand-down segment.
 *
 * Additionally, for 64bit environments:
 *
 * - A non-canonical virtual address.
 * - A virtual address which, when combined with a segment base, becomes
 *   non-canonical.
 *
 * This part of the test is applicable to non-paged environments.
 *
 * @todo Find a better way dealing with forced emulation.  Xen currently
 * crashes the domain if it finds an invlpg instruction while in hap mode,
 * which is unhelpful when trying to test behaviour.
 *
 * @todo Come up with some sensible way of running this test in both shadow
 * and hap mode.
 *
 * @see tests/invlpg/main.c
 */
#include <xtf.h>

#include <arch/x86/decode.h>
#include <arch/x86/desc.h>
#include <arch/x86/msr-index.h>
#include <arch/x86/symbolic-const.h>

/* Swizzle to easily change all invlpg instructions to being emulated. */
#if 0
#define _ASM_MAYBE_XEN_FEP _ASM_XEN_FEP
#else
#define _ASM_MAYBE_XEN_FEP
#endif

/**
 * Custom extable handler, linked to all `invlpg` instruction which are
 * expected not to fault.
 */
static bool __used ex_fail(struct cpu_regs *regs,
                           const struct extable_entry *ex)
{
    char buf[16];

    x86_exc_decode_ec(buf, ARRAY_SIZE(buf),
                      regs->entry_vector, regs->error_code);

    xtf_failure("    Fail: Unexpected %s[%s]\n",
                x86_exc_short_name(regs->entry_vector), buf);

    regs->ip = ex->fixup;
    return true;
}

/** Are both the Accessed and Dirty bits are set in a pagetable entry? */
static bool test_ad(uint64_t pte)
{
    return (pte & _PAGE_AD) == _PAGE_AD;
}

static unsigned int invlpg_refill(void)
{
    asm volatile ("mov %[zero], 0x1000;\n\t"   /* Force TLB fill. */
                  "mov %[zero], 0x2000;\n\t"
                  "andb $~%c[ad], %[pte1];\n\t" /* Clear A/D bits. */
                  "andb $~%c[ad], %[pte2];\n\t"
                  _ASM_MAYBE_XEN_FEP
                  "1: invlpg (0x1000); 2:\n\t" /* Invalidate one page only. */
                  _ASM_EXTABLE_HANDLER(1b, 2b, ex_fail)
                  "mov %[zero], 0x1000;\n\t"   /* Expect refill. */
                  "mov %[zero], 0x2000;\n\t"   /* Expect no refill. */
                  :
                  : [zero] "q" (0),
                    [ad]   "i" (_PAGE_AD),
                    [pte1] "m" (pae_l1_identmap[1]),
                    [pte2] "m" (pae_l1_identmap[2])
                  : "memory");

    return ((test_ad(pae_l1_identmap[1]) << 0) |
            (test_ad(pae_l1_identmap[2]) << 1));
}

static unsigned int invlpg_fs_refill(void)
{
    asm volatile ("mov %[zero], 0x1000;\n\t"  /* Force TLB fill. */
                  "mov %[zero], 0x2000;\n\t"
                  "andb $~%c[ad], %[pte1];\n\t" /* Clear A/D bits. */
                  "andb $~%c[ad], %[pte2];\n\t"
                  _ASM_MAYBE_XEN_FEP
                  "1: invlpg %%fs:(0x1000); 2:\n\t" /* Invalidate one page only. */
                  _ASM_EXTABLE_HANDLER(1b, 2b, ex_fail)
                  "mov %[zero], 0x1000;\n\t"  /* Expect one TLB entry to refil, */
                  "mov %[zero], 0x2000;\n\t"  /* depending on %fs base.*/
                  :
                  : [zero] "q" (0),
                    [ad]   "i" (_PAGE_AD),
                    [pte1] "m" (pae_l1_identmap[1]),
                    [pte2] "m" (pae_l1_identmap[2])
                  : "memory");

    return ((test_ad(pae_l1_identmap[1]) << 0) |
            (test_ad(pae_l1_identmap[2]) << 1));
}

static void run_tlb_refill_test(unsigned int (*fn)(void), unsigned int expect)
{
    unsigned int ret, tries = 20;

    do
    {
        ret = fn();
        /*
         * Tests are racy if a vcpu reschedule happens in the asm blocks, as
         * the TLB on the new vcpu will be empty and extra refills will occur.
         * If a refil of both entries occurs, repeat the test quickly several
         * times in the hope that one doesn't hit a reschedule point.
         */
    } while ( ret == 3 && --tries );

    switch ( ret )
    {
    case 0:
        return xtf_failure("    Fail: No TLB refill at all\n");
        break;

    case 1: case 2:
        printk("    TLB refill of %#x\n", ret * 0x1000);

        if ( ret != expect )
            xtf_failure("    Fail: Expected mapping %#x to have been refilled\n",
                        expect * 0x1000);
        break;

    case 3:
        printk("    TLB refill of both 0x1000 and 0x2000\n");
        break;

    default:
        return xtf_failure("    Fail: Unexpected return value %u\n", ret);
    }
}

static const struct tlb_refill_fs_test
{
    const char *desc;
    unsigned int mapping;
    user_desc seg;

} tlb_refill_fs_tests[] =
{
    { "(base 0x0)", 1,
      INIT_GDTE_SYM(0, 0xfffff, P, S, G, A, DATA, DPL0, B, W),
    },

    { "(base 0x0, limit 0x1)", 1,
      INIT_GDTE_SYM(0, 1, P, S, A, DATA, DPL0, B, W),
    },

    { "(base 0x1000)", 2,
      INIT_GDTE_SYM(0x1000, 0xfffff, P, S, G, A, DATA, DPL0, B, W),
    },

    { "(base 0x1000, limit 0x1001)", 2,
      INIT_GDTE_SYM(0x1000, 0x1001, P, S, A, DATA, DPL0, B, W),
    },
};

static void test_tlb_refill(void)
{
    unsigned int i;

    printk("Testing 'invlpg (0x1000)' with segment bases\n");

    printk("  Test: No segment\n");
    run_tlb_refill_test(invlpg_refill, 1);

    for ( i = 0; i < ARRAY_SIZE(tlb_refill_fs_tests); ++i )
    {
        const struct tlb_refill_fs_test *t = &tlb_refill_fs_tests[i];

        printk("  Test: %%fs %s\n", t->desc);
        gdt[GDTE_AVAIL0] = t->seg;
        write_fs(GDTE_AVAIL0 << 3);
        run_tlb_refill_test(invlpg_fs_refill, t->mapping);
    }
}

static void invlpg_checked(unsigned long linear)
{
    asm volatile (_ASM_MAYBE_XEN_FEP
                  "1: invlpg (%0); 2:"
                  _ASM_EXTABLE_HANDLER(1b, 2b, ex_fail)
                  :: "r" (linear));
}

static void invlpg_fs_checked(unsigned long linear)
{
    asm volatile (_ASM_MAYBE_XEN_FEP
                  "1: invlpg %%fs:(%0); 2:"
                  _ASM_EXTABLE_HANDLER(1b, 2b, ex_fail)
                  :: "r" (linear));
}

static void test_no_fault(void)
{
    printk("Testing 'invlpg' in normally-faulting conditions\n");

    printk("  Test: Mapped address\n");
    invlpg_checked((unsigned long)&test_main);

    printk("  Test: Unmapped address\n");
    invlpg_checked(0);

    printk("  Test: NULL segment override\n");
    write_fs(0);
    invlpg_fs_checked(0);

    printk("  Test: Past segment limit\n");
    gdt[GDTE_AVAIL0] = (typeof(*gdt))INIT_GDTE_SYM(0, 1, COMMON, DATA, DPL0, B, W);
    write_fs(GDTE_AVAIL0 << 3);
    invlpg_fs_checked(0x2000);

    printk("  Test: Before expand-down segment limit\n");
    gdt[GDTE_AVAIL0] = (typeof(*gdt))INIT_GDTE_SYM(0, 1, COMMON, DATA, DPL0, B, W, E);
    write_fs(GDTE_AVAIL0 << 3);
    invlpg_fs_checked(0);

#if CONFIG_PAGING_LEVELS >= 4
    printk("  Test: Noncanonical address\n");
    invlpg_checked(0x800000000000ULL);

    printk("  Test: Noncanonical including segment base\n");
    write_fs(0);
    wrmsr(MSR_FS_BASE, (1UL << 47) - 1);
    invlpg_fs_checked((1UL << 47) - 1);
#endif
}

void test_main(void)
{
    if ( CONFIG_PAGING_LEVELS > 0 )
        test_tlb_refill();
    test_no_fault();

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
