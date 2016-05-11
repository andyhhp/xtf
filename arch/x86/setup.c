#include <xtf/lib.h>
#include <xtf/console.h>
#include <xtf/hypercall.h>
#include <xtf/extable.h>

#include <arch/x86/desc.h>
#include <arch/x86/lib.h>
#include <arch/x86/mm.h>
#include <arch/x86/traps.h>

/*
 * XTF Stack layout:
 *
 * boot_stack[page 2] Exception entrypoints
 * boot_stack[page 1] Top of work stack
 */
uint8_t boot_stack[2 * PAGE_SIZE] __aligned(PAGE_SIZE);

const char *environment_description = ENVIRONMENT_DESCRIPTION;

#ifdef CONFIG_PV
/* Filled in by head_pv.S */
start_info_t *start_info = NULL;
#endif

/*
 * PV guests should have hypercalls set up by the domain builder, due to the
 * HYPERCALL_PAGE ELFNOTE being filled.  HVM guests have to locate the
 * hypervisor cpuid leaves to find correct MSR to requst that Xen writes a
 * hypercall page.
 */
static void init_hypercalls(void)
{
#ifdef CONFIG_HVM
    uint32_t eax, ebx, ecx, edx, base;
    bool found = false;

    for ( base = XEN_CPUID_FIRST_LEAF;
          base < XEN_CPUID_FIRST_LEAF + 0x10000; base += 0x100 )
    {
        cpuid(base, &eax, &ebx, &ecx, &edx);

        if ( (ebx == XEN_CPUID_SIGNATURE_EBX) &&
             (ecx == XEN_CPUID_SIGNATURE_ECX) &&
             (edx == XEN_CPUID_SIGNATURE_EDX) &&
             ((eax - base) >= 2) )
        {
            found = true;
            break;
        }
    }

    if ( !found )
        panic("Unable to locate Xen CPUID leaves\n");

    cpuid(base + 2, &eax, &ebx, &ecx, &edx);
    wrmsr(ebx, (unsigned long)&hypercall_page);
    barrier();
#endif /* CONFIG_HVM */

    /*
     * Confirm that the `ret` poision has been overwritten with a real
     * hypercall page.  At the time of writing, a legitimate hypercall page
     * should start with `movl $0, %eax` or `0xb8 imm32`.
     */
    if ( hypercall_page[0] == 0xc3 )
        panic("Hypercall page not initialised correctly\n");
}

static void setup_pv_console(void)
{
    xencons_interface_t *cons_ring;
    evtchn_port_t cons_evtchn;

#if defined(CONFIG_PV)
    cons_ring = mfn_to_virt(start_info->console.domU.mfn);
    cons_evtchn = start_info->console.domU.evtchn;
#elif defined(CONFIG_HVM)
    {
        uint64_t raw_pfn, raw_evtchn;

        if ( hvm_get_param(HVM_PARAM_CONSOLE_PFN, &raw_pfn) != 0 ||
             hvm_get_param(HVM_PARAM_CONSOLE_EVTCHN, &raw_evtchn) != 0 )
            return;

        cons_ring = pfn_to_virt(raw_pfn);
        cons_evtchn = raw_evtchn;
    }
#endif

    init_pv_console(cons_ring, cons_evtchn);
}

#if defined(CONFIG_HVM)
static void qemu_console_write(const char *buf, size_t len)
{
    asm volatile("rep; outsb"
                 : "+S" (buf), "+c" (len)
                 : "d" (0x12));
}
#endif

static void xen_console_write(const char *buf, size_t len)
{
    hypercall_console_write(buf, len);
}

void arch_setup(void)
{
#if defined(CONFIG_HVM)
    register_console_callback(qemu_console_write);
#endif

    register_console_callback(xen_console_write);

    arch_init_traps();

    init_hypercalls();

    sort_extable();

    setup_pv_console();
}

/*
 * Common test setup:
 *
 * xtf_has_* indicates the availabiliy of options which require runtime
 * detection.
 */
bool xtf_has_fep = false;

void test_setup(void)
{
    /*
     * Attempt to use the forced emulation prefix to set the value of
     * xtf_has_fep to the value of 1.  Use the exception table to compensate
     * for the #UD exception if FEP is not available.
     */
    asm volatile ("xor %0, %0;"
                  "1:" _ASM_XEN_FEP
                  "mov $1, %0;"
                  "2:"
                  _ASM_EXTABLE(1b, 2b)
                  : "=q" (xtf_has_fep));
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
