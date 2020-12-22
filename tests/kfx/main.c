/**
 * @file tests/kfx/main.c
 * @ref test-kfx
 *
 * @page test-kfx kfx
 *
 * @todo Docs for test-kfx
 *
 * @see tests/kfx/main.c
 */
#include <xtf.h>

const char test_title[] = "Test kfx";

static struct xen_domctl op;

static void kfx_fuzz_setup(void *_p, size_t s)
{
    intptr_t p = (intptr_t)_p;
    unsigned int tmp;

    asm volatile ("cpuid"
                  : "=a" (tmp), "=c" (tmp)
                  : "a" (0x13371337), "c" (s)
                  : "bx", "dx");

    asm volatile ("cpuid"
                  : "=a" (tmp), "=c" (tmp)
#ifdef __x86_64__
                  : "a" (p >> 32),
#else
                  : "a" (0),
#endif
                    "c" (p)
                  : "bx", "dx");
}

static void kfx_fuzz_stop(void)
{
    cpuid_eax(0x13371337);
}

void test_main(void)
{
    int interface_version = xtf_probe_domctl_interface_version();
    if ( interface_version < 0 )
        return xtf_error("Failed to get domctl version\n");

    printk("Domctl version: %#x. Struct @ %p size %zu\n",
           interface_version, &op, sizeof(op));

    kfx_fuzz_setup(&op, sizeof(op));

    op.cmd = XEN_DOMCTL_createdomain,
    op.interface_version = interface_version;

    int rc = hypercall_domctl(&op);
    if ( rc == 0 )
    {
        op.cmd = XEN_DOMCTL_destroydomain;
        hypercall_domctl(&op);
    }

    kfx_fuzz_stop();

    xtf_success("Fuzzing done\n");
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
