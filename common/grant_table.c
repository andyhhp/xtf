/**
 * @file common/grant_table.c
 *
 * A driver for the Xen Grant Table interface.
 */
#include <xtf/grant_table.h>
#include <xtf/lib.h>

uint8_t gnttab_raw[] __page_aligned_bss;
extern grant_entry_v1_t gnttab_v1[] __alias("gnttab_raw");
extern grant_entry_v2_t gnttab_v2[] __alias("gnttab_raw");

const char *gntst_strerror(int err)
{
    static const char *const errstr[] = GNTTABOP_error_msgs;
    unsigned int idx = -err;

    return idx < ARRAY_SIZE(errstr) ? errstr[idx] : "unknown";
}

int xtf_init_grant_table(unsigned int version)
{
    int rc = 0;

    static bool set_version_unavailable;
    if ( !set_version_unavailable )
    {
        struct gnttab_set_version ver = { version };

        rc = hypercall_grant_table_op(GNTTABOP_set_version, &ver, 1);

        if ( rc == -ENOSYS )
            /* Hypercall unavailable on older versions of Xen. */
            set_version_unavailable = true;

        else if ( rc )
        {
            printk("%s() GNTTABOP_set_version failed: rc %d\n", __func__, rc);
            return -EIO;
        }
    }

    if ( set_version_unavailable && version != 1 )
        /* Sufficiently old Xen which only knows about gnttab v1. */
        return -ENODEV;

    static bool gnttab_mapped;
    if ( !gnttab_mapped )
    {
        rc = arch_map_gnttab();

        if ( !rc )
            gnttab_mapped = true;
    }

    return rc;
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
