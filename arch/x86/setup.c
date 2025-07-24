#include <xtf/lib.h>
#include <xtf/hypercall.h>
#include <xtf/extable.h>
#include <xtf/report.h>
#include <xtf/xenbus.h>

#include <arch/cpuid.h>
#include <arch/desc.h>
#include <arch/lib.h>
#include <arch/mm.h>
#include <arch/symbolic-const.h>
#include <arch/traps.h>

/*
 * XTF Stack layout:
 *
 * boot_stack[page 3] Emergency entrypoint
 * boot_stack[page 2] Exception entrypoints
 * boot_stack[page 1] Top of work stack
 */
uint8_t boot_stack[3 * PAGE_SIZE] __page_aligned_bss;
uint8_t user_stack[PAGE_SIZE] __user_page_aligned_bss;

uint32_t x86_features[FSCAPINTS];
enum x86_vendor x86_vendor;
unsigned int max_leaf, max_extd_leaf;
unsigned int x86_family, x86_model, x86_stepping;
unsigned int maxphysaddr, maxvirtaddr;

const char environment_description[] = ENVIRONMENT_DESCRIPTION;

shared_info_t shared_info __page_aligned_bss;

static void collect_cpuid(cpuid_count_fn_t cpuid_fn)
{
    unsigned int tmp, eax, ebx, ecx, edx, addr = 0;

    cpuid_fn(0, 0, &max_leaf, &ebx, &ecx, &edx);

    if ( ebx == 0x756e6547u &&      /* "GenuineIntel" */
         ecx == 0x6c65746eu &&
         edx == 0x49656e69u )
        x86_vendor = X86_VENDOR_INTEL;

    else if ( ebx == 0x68747541u && /* "AuthenticAMD" */
              ecx == 0x444d4163u &&
              edx == 0x69746e65u )
        x86_vendor = X86_VENDOR_AMD;

    else
        x86_vendor = X86_VENDOR_UNKNOWN;

    if ( max_leaf >= 1 )
    {
        cpuid_fn(1, 0, &eax, &tmp,
                 &x86_features[FEATURESET_1c],
                 &x86_features[FEATURESET_1d]);

        x86_stepping = (eax >> 0) & 0xf;
        x86_model    = (eax >> 4) & 0xf;
        x86_family   = (eax >> 8) & 0xf;

        if ( (x86_family == 0xf) ||
             (x86_vendor == X86_VENDOR_INTEL && x86_family == 0x6) )
            x86_model |= ((eax >> 16) & 0xf) << 4;

        if ( x86_family == 0xf )
            x86_family += (eax >> 20) & 0xff;
    }
    if ( max_leaf >= 7 )
        cpuid_fn(7, 0, &tmp,
                 &x86_features[FEATURESET_7b0],
                 &x86_features[FEATURESET_7c0],
                 &x86_features[FEATURESET_7d0]);
    if ( max_leaf >= 0xd )
        cpuid_fn(0xd, 0,
                 &x86_features[FEATURESET_Da1],
                 &tmp, &tmp, &tmp);

    cpuid_fn(0x80000000, 0, &max_extd_leaf, &tmp, &tmp, &tmp);

    if ( (max_extd_leaf >> 16) == 0x8000 )
    {
        if ( max_extd_leaf >= 0x80000001 )
            cpuid_fn(0x80000001, 0, &tmp, &tmp,
                     &x86_features[FEATURESET_e1c],
                     &x86_features[FEATURESET_e1d]);
        if ( max_extd_leaf >= 0x80000007 )
            cpuid_fn(0x80000007, 0, &tmp, &tmp, &tmp,
                     &x86_features[FEATURESET_e7d]);
        if ( max_extd_leaf >= 0x80000008 )
            cpuid_fn(0x80000008, 0, &addr,
                     &x86_features[FEATURESET_e8b],
                     &tmp, &tmp);
    }

    maxphysaddr = (addr & 0xff) ?: 36;
    maxvirtaddr = ((addr >> 8) & 0xff) ?: 32;
}

/*
 * Find the Xen CPUID leaves.  They may be at 0x4000_0000, or at 0x4000_0100
 * if Xen is e.g. providing a viridian interface to the guest too.
 */
static unsigned int find_xen_leaves(void)
{
    static unsigned int leaves;

    if ( leaves )
        return leaves;

    for ( unsigned int l = XEN_CPUID_FIRST_LEAF;
          l < XEN_CPUID_FIRST_LEAF + 0x10000; l += 0x100 )
    {
        unsigned int eax, ebx, ecx, edx;

        cpuid(l, &eax, &ebx, &ecx, &edx);

        if ( (ebx == XEN_CPUID_SIGNATURE_EBX) &&
             (ecx == XEN_CPUID_SIGNATURE_ECX) &&
             (edx == XEN_CPUID_SIGNATURE_EDX) &&
             ((eax - l) >= 2) )
        {
            leaves = l;
            return l;
        }
    }

    panic("Unable to locate Xen CPUID leaves\n");
}

/*
 * PV guests should have hypercalls set up by the domain builder, due to the
 * HYPERCALL_PAGE ELFNOTE being filled.  HVM guests have to locate the
 * hypervisor cpuid leaves to find correct MSR to requst that Xen writes a
 * hypercall page.
 */
static void init_hypercalls(void)
{
    if ( IS_DEFINED(CONFIG_HVM) )
    {
        uint32_t eax, ebx, ecx, edx;
        unsigned int base = find_xen_leaves();

        cpuid(base + 2, &eax, &ebx, &ecx, &edx);
        wrmsr(ebx, _u(hypercall_page));
        barrier();
    }

    /*
     * Confirm that the `ret` poision has been overwritten with a real
     * hypercall page.  At the time of writing, a legitimate hypercall page
     * should start with `movl $0, %eax` or `0xb8 imm32`.
     */
    if ( hypercall_page[0] == 0xc3 )
        panic("Hypercall page not initialised correctly\n");
}

static bool is_initdomain(void)
{
    if ( IS_DEFINED(CONFIG_PV) )
        return pv_start_info->flags & SIF_INITDOMAIN;
    else
        return pvh_start_info && pvh_start_info->flags & SIF_INITDOMAIN;
}

static void setup_pv_console(void)
{
    xencons_interface_t *cons_ring;
    evtchn_port_t cons_evtchn;

    if ( IS_DEFINED(CONFIG_PV) )
    {
        cons_ring = mfn_to_virt(pv_start_info->console.domU.mfn);
        cons_evtchn = pv_start_info->console.domU.evtchn;
    }
    else /* HVM */
    {
        uint64_t raw_pfn, raw_evtchn;

        if ( hvm_get_param(HVM_PARAM_CONSOLE_PFN, &raw_pfn) != 0 ||
             hvm_get_param(HVM_PARAM_CONSOLE_EVTCHN, &raw_evtchn) != 0 )
            return;

        cons_ring = pfn_to_virt(raw_pfn);
        cons_evtchn = raw_evtchn;
    }

    init_pv_console(cons_ring, cons_evtchn);
}

static void setup_xenbus(void)
{
    xenbus_interface_t *xb_ring;
    evtchn_port_t xb_port;

    if ( IS_DEFINED(CONFIG_PV) )
    {
        xb_ring = mfn_to_virt(pv_start_info->store_mfn);
        xb_port = pv_start_info->store_evtchn;
    }
    else /* HVM */
    {
        uint64_t raw_pfn, raw_evtchn;
        int rc;

        rc = hvm_get_param(HVM_PARAM_STORE_PFN, &raw_pfn);
        if ( rc )
            panic("Failed to get XenStore PFN: %d\n", rc);

        rc = hvm_get_param(HVM_PARAM_STORE_EVTCHN, &raw_evtchn);
        if ( rc )
            panic("Failed to get XenStore evtchn: %d\n", rc);

        xb_ring = pfn_to_virt(raw_pfn);
        xb_port = raw_evtchn;
    }

    init_xenbus(xb_ring, xb_port);
}

static void map_shared_info(void)
{
    int rc;

    if ( IS_DEFINED(CONFIG_HVM) )
    {
        struct xen_add_to_physmap xatp = {
            .domid = DOMID_SELF,
            .space = XENMAPSPACE_shared_info,
            .idx = 0,
            .gfn = virt_to_gfn(&shared_info),
        };

        rc = hypercall_memory_op(XENMEM_add_to_physmap, &xatp);
    }
    else /* PV */
        rc = hypercall_update_va_mapping(
            _u(&shared_info), pv_start_info->shared_info | PF_SYM(RW, P),
            UVMF_INVLPG);

    if ( rc )
        panic("Failed to map shared_info: %d\n", rc);
}

static void qemu_console_write(const char *buf, size_t len)
{
    asm volatile("rep outsb"
                 : "+S" (buf), "+c" (len)
                 : "d" (0x12));
}

static void xen_console_write(const char *buf, size_t len)
{
    hypercall_console_write(buf, len);
}

void arch_setup(void)
{
    if ( IS_DEFINED(CONFIG_HVM) && !pvh_start_info )
        register_console_callback(qemu_console_write);

    register_console_callback(xen_console_write);

    collect_cpuid(IS_DEFINED(CONFIG_PV) ? pv_cpuid_count : cpuid_count);

    sort_extable();

    arch_init_traps();

    init_hypercalls();

    if ( !is_initdomain() )
    {
        setup_pv_console();
        setup_xenbus();
    }

    map_shared_info();
}

/*
 * Common test setup:
 *
 * xtf_has_* indicates the availabiliy of options which require runtime
 * detection.
 */
bool xtf_has_fep = false;

/*
 * Default weak settings.
 *
 * test_wants_* indicates default settings for the framework, which may be
 * overridden by individual tests by providing non-weak variables.
 *
 * test_needs_* indicates options required by the test, without which the test
 * isn't worth running.
 */
bool __weak test_needs_fep = false;

void test_setup(void)
{
    /*
     * Attempt to use the forced emulation prefix to set the value of
     * xtf_has_fep to the value of 1.  Use the exception table to compensate
     * for the #UD exception if FEP is not available.
     */
    asm volatile ("1:" _ASM_XEN_FEP "mov $1, %[fep];"
                  "2:"
                  _ASM_EXTABLE(1b, 2b)
                  : [fep] "=q" (xtf_has_fep)
                  : "0" (false));

    if ( test_needs_fep && !xtf_has_fep )
    {
        printk("FEP unavailable, but needed by test. (Is Xen support\n");
        return xtf_skip("compiled in, and booted with 'hvm_fep'?)\n");
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
