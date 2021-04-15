/**
 * @file arch/x86/grant_table.c
 *
 * %x86 specific bits of Grant Table handling
 */
#include <xtf/grant_table.h>
#include <xtf/hypercall.h>
#include <xtf/lib.h>

#include <arch/pagetable.h>
#include <arch/symbolic-const.h>

int arch_map_gnttab(void)
{
    unsigned int i;
    int rc = 0;

    /* Ensure gnttab_raw[] is a whole number of pages. */
    BUILD_BUG_ON(sizeof(gnttab_raw) % PAGE_SIZE);

    if ( IS_DEFINED(CONFIG_PV) )
    {
        unsigned long gnttab_gfns[sizeof(gnttab_raw) / PAGE_SIZE] = {};
        struct gnttab_setup_table setup = {
            .dom = DOMID_SELF,
            .nr_frames = ARRAY_SIZE(gnttab_gfns),
            .frame_list = gnttab_gfns,
        };

        rc = hypercall_grant_table_op(GNTTABOP_setup_table, &setup, 1);
        if ( rc || setup.status )
        {
            printk("%s() GNTTABOP_setup_table failed: rc %d, status %d: %s\n",
                   __func__, rc, setup.status, gntst_strerror(setup.status));
            return -EIO;
        }

        for ( i = 0; !rc && i < ARRAY_SIZE(gnttab_gfns); ++i )
            rc = hypercall_update_va_mapping(
                _u(&gnttab_raw[i * PAGE_SIZE]),
                pte_from_gfn(gnttab_gfns[i], PF_SYM(AD, RW, P)), UVMF_INVLPG);
    }
    else /* HVM */
    {
        struct xen_add_to_physmap xatp = {
            .domid = DOMID_SELF,
            .space = XENMAPSPACE_grant_table,
            .idx = 0,
            .gfn = virt_to_gfn(gnttab_raw),
        };

        for ( i = 0; !rc && i < (sizeof(gnttab_raw) / PAGE_SIZE);
              ++i, ++xatp.idx, ++xatp.gfn )
            rc = hypercall_memory_op(XENMEM_add_to_physmap, &xatp);
    }

    if ( rc )
    {
        printk("%s() Failed to map gnttab[%u]: %d\n", __func__, i, rc);
        return -EIO;
    }

    return 0;
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
