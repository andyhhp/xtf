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
    struct gnttab_set_version ver = { version };

    int rc = hypercall_grant_table_op(GNTTABOP_set_version, &ver, 1);

    if ( rc == -ENOSYS )
        /* Sufficiently old Xen which doesn't support gnttab v2. */
        return -ENODEV;

    if ( rc )
    {
        printk("%s() GNTTABOP_set_version failed: rc %d\n", __func__, rc);
        return -EIO;
    }

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
