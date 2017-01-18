/**
 * @file tests/vmfunc/main.c
 * @ref test-vmfunc
 *
 * @page test-vmfunc vmfunc
 *
 * @todo Docs for test-vmfunc
 *
 * @see tests/vmfunc/main.c
 */
#include <xtf.h>

const char test_title[] = "Test vmfunc";

/* HVMOP_altp2m: perform altp2m state operations */
#define HVMOP_altp2m 25

#define HVMOP_ALTP2M_INTERFACE_VERSION 0x00000001

struct xen_hvm_altp2m_domain_state {
    /* IN or OUT variable on/off */
    uint8_t state;
};
typedef struct xen_hvm_altp2m_domain_state xen_hvm_altp2m_domain_state_t;

struct xen_hvm_altp2m_vcpu_enable_notify {
    uint32_t vcpu_id;
    uint32_t pad;
    /* #VE info area gfn */
    uint64_t gfn;
};
typedef struct xen_hvm_altp2m_vcpu_enable_notify xen_hvm_altp2m_vcpu_enable_notify_t;

struct xen_hvm_altp2m_view {
    /* IN/OUT variable */
    uint16_t view;
    /* Create view only: default access type
     * NOTE: currently ignored */
    uint16_t hvmmem_default_access; /* xenmem_access_t */
};
typedef struct xen_hvm_altp2m_view xen_hvm_altp2m_view_t;

struct xen_hvm_altp2m_mem_access {
    /* view */
    uint16_t view;
    /* Memory type */
    uint16_t access; /* xenmem_access_t */
    uint32_t pad;
    /* gfn */
    uint64_t gfn;
};
typedef struct xen_hvm_altp2m_set_mem_access xen_hvm_altp2m_set_mem_access_t;

struct xen_hvm_altp2m_change_gfn {
    /* view */
    uint16_t view;
    uint16_t pad1;
    uint32_t pad2;
    /* old gfn */
    uint64_t old_gfn;
    /* new gfn, INVALID_GFN (~0UL) means revert */
    uint64_t new_gfn;
};

struct xen_hvm_altp2m_suppress_ve {
    uint16_t view;
    uint8_t suppress_ve; /* Boolean type. */
    uint8_t pad1;
    uint32_t pad2;
    uint64_t gfn;
};

struct xen_hvm_altp2m_op {
    uint32_t version;   /* HVMOP_ALTP2M_INTERFACE_VERSION */
    uint32_t cmd;
    /* Get/set the altp2m state for a domain */
    #define HVMOP_altp2m_get_domain_state     1
    #define HVMOP_altp2m_set_domain_state     2
    /* Set the current VCPU to receive altp2m event notifications */
    #define HVMOP_altp2m_vcpu_enable_notify   3
    /* Create a new view */
    #define HVMOP_altp2m_create_p2m           4
    /* Destroy a view */
    #define HVMOP_altp2m_destroy_p2m          5
    /* Switch view for an entire domain */
    #define HVMOP_altp2m_switch_p2m           6
    /* Notify that a page of memory is to have specific access types */
    #define HVMOP_altp2m_set_mem_access       7
    /* Change a p2m entry to have a different gfn->mfn mapping */
    #define HVMOP_altp2m_change_gfn           8
    /* Set the "Suppress #VE" bit on a page */
    #define HVMOP_altp2m_set_suppress_ve      10
    domid_t domain;
    uint16_t pad1;
    uint32_t pad2;
    union {
        struct xen_hvm_altp2m_domain_state       domain_state;
        struct xen_hvm_altp2m_vcpu_enable_notify enable_notify;
        struct xen_hvm_altp2m_view               view;
        struct xen_hvm_altp2m_mem_access         mem_access;
        struct xen_hvm_altp2m_change_gfn         change_gfn;
        struct xen_hvm_altp2m_suppress_ve        suppress_ve;
        uint8_t pad[64];
    } u;
};

static uint8_t ve_frame[PAGE_SIZE] __page_aligned_bss;

void test_main(void)
{
    struct xen_hvm_altp2m_op ap2m = {
        .version = HVMOP_ALTP2M_INTERFACE_VERSION,
        .domain = DOMID_SELF,
    };

    printk("ap2m get\n");
    ap2m.cmd = HVMOP_altp2m_get_domain_state;
    int rc = hypercall_hvm_op(HVMOP_altp2m, &ap2m);
    printk("ap2m get: %d, val %u\n", rc, ap2m.u.domain_state.state);

    ap2m.cmd = HVMOP_altp2m_set_domain_state;
    ap2m.u.domain_state = (struct xen_hvm_altp2m_domain_state){ .state = 1 };

    printk("ap2m set\n");
    rc = hypercall_hvm_op(HVMOP_altp2m, &ap2m);
    printk("ap2m set: %d\n", rc);

    ap2m.cmd = HVMOP_altp2m_get_domain_state;

    printk("ap2m get\n");
    rc = hypercall_hvm_op(HVMOP_altp2m, &ap2m);
    printk("ap2m get: %d, val %u\n", rc, ap2m.u.domain_state.state);

    ap2m.cmd = HVMOP_altp2m_create_p2m;
    ap2m.u.view = (struct xen_hvm_altp2m_view){ .view = 0 };

    printk("ap2m create\n");
    rc = hypercall_hvm_op(HVMOP_altp2m, &ap2m);
    printk("ap2m create: %d, val %u\n", rc, ap2m.u.view.view);

    unsigned int view = ap2m.u.view.view;

    ap2m.cmd = HVMOP_altp2m_vcpu_enable_notify;
    ap2m.u.enable_notify = (struct xen_hvm_altp2m_vcpu_enable_notify) {
        .vcpu_id = 0,
        .gfn = virt_to_gfn(ve_frame),
    };

    printk("ap2m enable notify\n");
    rc = hypercall_hvm_op(HVMOP_altp2m, &ap2m);
    printk("ap2m enable notify: %d\n", rc);

    printk("Switching to view %u\n", view);
    asm volatile ("vmfunc" :: "a" (0), "c" (view));
    printk("Switching to view %u done\n", view);

    ap2m.cmd = HVMOP_altp2m_set_mem_access;
    ap2m.u.mem_access = (struct xen_hvm_altp2m_mem_access){
        .view = view,
        .access = /* XENMEM_access_r */ 1,
        .gfn = 1,
    };

    printk("ap2m set access\n");
    rc = hypercall_hvm_op(HVMOP_altp2m, &ap2m);
    printk("ap2m set access ro: %d\n", rc);

    ap2m.cmd = HVMOP_altp2m_set_suppress_ve;
    ap2m.u.suppress_ve = (struct xen_hvm_altp2m_suppress_ve){
        .view = view,
        .suppress_ve = 0,
        .gfn = 1,
    };

    printk("ap2m allow #VE\n");
    rc = hypercall_hvm_op(HVMOP_altp2m, &ap2m);
    printk("ap2m allow #VE: %d\n", rc);

    unsigned long extent = virt_to_gfn(ve_frame);
    struct xen_memory_reservation mr = {
        .extent_start = &extent,
        .nr_extents = 1,
        .domid = DOMID_SELF,
    };

    if ( hypercall_memory_op(XENMEM_decrease_reservation, &mr) != 1 )
        printk("Failed to balloon out\n");
    else
        printk("Ok\n");

    printk("Trying read:\n");
    (void)ACCESS_ONCE(*(char *)KB(4));

    printk("Trying write:\n");
    ACCESS_ONCE(*(char *)KB(4)) = 0;

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
