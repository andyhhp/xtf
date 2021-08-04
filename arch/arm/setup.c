/**
 * @file arch/arm/setup.c
 *
 * Early bringup code for arm.
 */
#include <xtf/hypercall.h>
#include <xtf/framework.h>
#include <arch/pl011.h>
#include <arch/mm.h>

const char environment_description[] = ENVIRONMENT_DESCRIPTION;

/* Structure to store boot arguments */
struct init_data
{
    uint64_t phys_offset;
    void *fdt;
} boot_data;

static void setup_console(void)
{
#ifdef CONFIG_PL011_UART
#ifndef CONFIG_PL011_EARLY_PRINTK
    /* Initialize UART */
    pl011_init();
#endif
    /* Use PL011 UART to print messages */
    register_console_callback(pl011_console_write);
#else
    /* Use Xen console to print messages */
    register_console_callback(hypercall_console_write);
#endif
}

#ifdef CONFIG_MMU
static void setup_pv_console(void)
{
    xencons_interface_t *cons_ring;
    evtchn_port_t cons_evtchn;
    uint64_t raw_ev = 0, raw_pfn = 0, phys, pfn;

    if (hvm_get_param(HVM_PARAM_CONSOLE_EVTCHN, &raw_ev) != 0 ||
        hvm_get_param(HVM_PARAM_CONSOLE_PFN, &raw_pfn) != 0)
        return;

    cons_evtchn = raw_ev;
    phys = pfn_to_phys(raw_pfn);
    pfn = set_fixmap(FIXMAP_PV_CONSOLE, phys, DESC_PAGE_TABLE_DEV);
    cons_ring = (xencons_interface_t *)pfn;

    init_pv_console(cons_ring, cons_evtchn);
}

static bool is_initdomain(void)
{
    xen_feature_info_t fi;
    int ret;

    fi.submap_idx = 0;
    ret = hypercall_xen_version(XENVER_get_features, &fi);

    if (ret)
        panic("Failed to obtain Xen features. ret=%d\n", ret);

    if (fi.submap & (1 << XENFEAT_dom0))
        return true;

    return false;
}
#endif

void arch_setup(void)
{
    setup_console();
#ifdef CONFIG_MMU
    setup_mm(boot_data.phys_offset);
    if (!is_initdomain())
        setup_pv_console();
#endif
}

void test_setup(void)
{
    /* Nothing to be done here for now */
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
