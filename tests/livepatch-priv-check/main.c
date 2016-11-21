/**
 * @file tests/livepatch-priv-check/main.c
 * @ref test-livepatch-priv-check
 *
 * @page test-livepatch-priv-check Live Patch Privilege Check
 *
 * Checks that Xen prevents using all live patching operations and
 * sub-operations from an unprivileged guest.
 *
 * @see tests/livepatch-check-priv/main.c
 */
#include <xtf.h>

const char test_title[] = "Live Patch Privilege Check";

static int sysctl_interface_version;

static void check_ret(const char *test, int rc)
{
    switch ( rc )
    {
    case -EPERM:
        printk("%s: Xen correctly denied Live Patch calls\n", test);
        break;

    case -ENOSYS:
        xtf_skip("%s: Live Patch support not detected\n", test);
        break;

    default:
        xtf_failure("Fail: %s: Unexpected return code %d\n", test, rc);
        break;
    }
}

#define TEST_NAME "foo"

static void test_upload(void)
{
    static uint8_t payload[PAGE_SIZE];
    xen_sysctl_t op =
    {
        .cmd = XEN_SYSCTL_livepatch_op,
        .interface_version = sysctl_interface_version,
        .u.livepatch = {
            .cmd = XEN_SYSCTL_LIVEPATCH_UPLOAD,
            .u.upload = {
                .name = {
                    .name.p = TEST_NAME,
                    .size = sizeof(TEST_NAME),
                },
                .size = PAGE_SIZE,
                .payload.p = payload,
            },
        },
    };

    check_ret(__func__, hypercall_sysctl(&op));
}

#define NR_PAYLOADS 8

static void test_list(void)
{
    char names[NR_PAYLOADS * XEN_LIVEPATCH_NAME_SIZE];
    uint32_t lengths[NR_PAYLOADS];

    xen_sysctl_t op =
    {
        .cmd = XEN_SYSCTL_livepatch_op,
        .interface_version = sysctl_interface_version,
        .u.livepatch = {
            .cmd = XEN_SYSCTL_LIVEPATCH_LIST,
            .u.list = {
                .idx = 0,
                .nr = NR_PAYLOADS,
                .name.p = names,
                .len.p = lengths,
            },
        },
    };

    check_ret(__func__, hypercall_sysctl(&op));
}

static void test_get(void)
{
    xen_sysctl_t op =
    {
        .cmd = XEN_SYSCTL_livepatch_op,
        .interface_version = sysctl_interface_version,
        .u.livepatch = {
            .cmd = XEN_SYSCTL_LIVEPATCH_GET,
            .u.get = {
                .name = {
                    .name.p = TEST_NAME,
                    .size = sizeof(TEST_NAME),
                },
            },
        },
    };

    check_ret(__func__, hypercall_sysctl(&op));
}

static void test_action(uint32_t action)
{
    xen_sysctl_t op =
    {
        .cmd = XEN_SYSCTL_livepatch_op,
        .interface_version = sysctl_interface_version,
        .u.livepatch = {
            .cmd = XEN_SYSCTL_LIVEPATCH_ACTION,
            .u.action = {
                .name = {
                    .name.p = TEST_NAME,
                    .size = sizeof(TEST_NAME),
                },
                .cmd = action,
                .timeout = 0,
            },
        },
    };

    check_ret(__func__, hypercall_sysctl(&op));
}

void test_main(void)
{
    sysctl_interface_version = xtf_probe_sysctl_interface_version();
    if ( sysctl_interface_version < 0 )
        return xtf_error("Failed to find sysctl interface version\n");

    test_upload();
    test_list();
    test_get();
    test_action(LIVEPATCH_ACTION_UNLOAD);
    test_action(LIVEPATCH_ACTION_REVERT);
    test_action(LIVEPATCH_ACTION_APPLY);
    test_action(LIVEPATCH_ACTION_REPLACE);

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
