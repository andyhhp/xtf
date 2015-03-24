#include <xtf/lib.h>
#include <xtf/console.h>
#include <xtf/hypercall.h>

#include <arch/x86/config.h>
#include <arch/x86/mm.h>

uint8_t boot_stack[PAGE_SIZE] __aligned(PAGE_SIZE);

#ifdef CONFIG_ENV_pv
/* Filled in by head_pv.S */
start_info_t *start_info = NULL;
#endif

/*
 * PV guests should have hypercalls set up by the domain builder, due to the
 * HYPERCALL_PAGE ELFNOTE being filled.
 */
static void init_hypercalls(void)
{
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

#if defined(CONFIG_ENV_pv)
    cons_ring = mfn_to_virt(start_info->console.domU.mfn);
    cons_evtchn = start_info->console.domU.evtchn;
#endif

    init_pv_console(cons_ring, cons_evtchn);
}

static void xen_console_write(const char *buf, size_t len)
{
    hypercall_console_write(buf, len);
}

void arch_setup(void)
{
    register_console_callback(xen_console_write);

    init_hypercalls();

    setup_pv_console();
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
