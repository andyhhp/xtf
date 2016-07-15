/**
 * @file tests/xsa-122/main.c
 * @ref test-xsa-122
 *
 * @page test-xsa-122 XSA-122
 *
 * Advisory: [XSA-122](http://xenbits.xen.org/xsa/advisory-122.html)
 *
 * Before XSA-122, Xen would fill a fixed size stack array with a
 * NUL-terminated string, and copy the entire array back to guest space.  This
 * leaks hypervisor stack rubble to the guest.
 *
 * This PoC makes the affected hypercalls, and checks for non-zero bytes in
 * the trailing space after the NUL terminator.
 *
 * The PoC is unable to distinguish between a fixed Xen, and zeroes happening
 * to be leaked from the stack.  In particular, it can incorrectly report
 * success if it is the first vcpu to run on a "fresh" pcpu after host reboot.
 * For added reliability, pin the PoC to a specific pcpu and run it twice.
 *
 * @see tests/xsa-122/main.c
 */
#include <xtf.h>

/*
 * Check a buffer of a specified size for non-NUL bytes following the string
 * NUL terminator.
 */
static void check_buffer(const char *ref, const char *buf, size_t buf_sz)
{
    unsigned i;
    size_t str_len = strnlen(buf, buf_sz);

    if ( str_len == buf_sz )
        return xtf_warning("    %s string not NUL terminated\n", ref);

    else if ( str_len == buf_sz - 1 )
        return; /* No trailing space after data. */

    for ( i = str_len + 1; buf[i] == '\0' && i < buf_sz; ++i )
        ; /* No action. */

    if ( i != buf_sz )
        xtf_failure("    '%s' has leaked stack at index %u\n", ref, i);
}

void test_main(void)
{
    long rc;

    printk("XSA-122 PoC\n");

    printk("XENVER_extraversion:\n");
    {
        xen_extraversion_t extra;

        memset(&extra, 0, sizeof(extra));

        rc = hypercall_xen_version(XENVER_extraversion, extra);
        if ( rc < 0 )
            return xtf_failure("XENVER_extraversion error %ld\n", rc);

        printk("  Got '%s'\n", extra);
        check_buffer("extraversion", extra, sizeof(extra));
    }

    printk("XENVER_compile_info:\n");
    {
        xen_compile_info_t ci;

        memset(&ci, 0, sizeof(ci));

        rc = hypercall_xen_version(XENVER_compile_info, &ci);
        if ( rc < 0 )
            return xtf_failure("XENVER_compile_info error %ld\n", rc);

        printk("  Got '%s'\n"
               "      '%s'\n"
               "      '%s'\n"
               "      '%s'\n",
               ci.compiler, ci.compile_by, ci.compile_domain, ci.compile_date);

        check_buffer("compiler", ci.compiler, sizeof(ci.compiler));
        check_buffer("compile_by", ci.compile_by, sizeof(ci.compile_by));
        check_buffer("compile_domain", ci.compile_domain, sizeof(ci.compile_domain));
        check_buffer("compile_date", ci.compile_date, sizeof(ci.compile_date));
    }

    printk("XENVER_changeset:\n");
    {
        xen_changeset_info_t ci;

        memset(&ci, 0, sizeof(ci));

        rc = hypercall_xen_version(XENVER_changeset, &ci);
        if ( rc < 0 )
            return xtf_failure("XENVER_changeset error %ld\n", rc);

        printk("  Got '%s'\n", ci);
        check_buffer("changeset_info", ci, sizeof(ci));
    }

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
