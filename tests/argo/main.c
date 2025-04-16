/**
 * @file tests/argo/main.c
 * @ref test-argo
 *
 * @page test-argo argo
 *
 * @todo Docs for test-argo
 *
 * @see tests/argo/main.c
 */
#include <xtf.h>

const char test_title[] = "Argo test";

/*
 * Xen enforces an upper limit of 16M for rings.  32 pages should suffice.
 */
static uint8_t ring_buffer[32 * PAGE_SIZE] __page_aligned_bss;
#define TEST_RING_NPAGES (sizeof(ring_buffer) / PAGE_SIZE)

static int probe_for_argo(domid_t own_domid)
{
    /* Attempt an Argo call to register a ring with invalid arguments */
    xen_argo_register_ring_t reg = {
        .aport      = 1,
        .partner_id = own_domid,
        .len        = 1, /* A 1-byte ring is never allowed */
    };
    int rc = hypercall_argo_op(XEN_ARGO_OP_register_ring, &reg, NULL, 0, 0);

    switch ( rc )
    {
    case -EINVAL: /* This is the response we are looking for */
        return 0;

        /* All below here are test exit conditions */
    case -ENOSYS:
        xtf_skip("Skip: Argo support has not been enabled in this hypervisor\n");
        break;
    case -EOPNOTSUPP:
        xtf_skip("Skip: Argo is not enabled at runtime for this hypervisor\n");
        break;
    case -ENODEV:
        xtf_skip("Skip: Argo is not enabled for this domain\n");
        break;

    case -EPERM:
        xtf_failure("Fail: Ring registration by this domain is not permitted\n");
        break;
    case 0:
        xtf_failure("Fail: An invalid ring register op was not rejected\n");
        break;
    default:
        xtf_failure("Fail: Unknown error %d for invalid ring registration\n", rc);
        break;
    }

    return -1;
}

/* notify: asks Xen for information about rings */
static int
test_notify_for_one_ring(domid_t query_domid, xen_argo_port_t query_port,
                         bool exists)
{
    struct {
        xen_argo_ring_data_t data;
        xen_argo_ring_data_ent_t ents[1];
    } notify = {
        .data = {
            .nent = ARRAY_SIZE(notify.ents),
        },
        .ents = {
            {
                .ring = {
                    .domain_id = query_domid,
                    .aport     = query_port,
                },
            },
        },
    };
    int rc = hypercall_argo_op(XEN_ARGO_OP_notify, &notify, NULL, 0, 0);

    if ( rc )
    {
        xtf_failure("Fail: Unexpected error code %d in notify test\n", rc);
        return -1;
    }

    if ( !exists )
    {
        /* No currently-defined flags should be set for a non-existent ring */
        if ( notify.ents[0].flags )
        {
            xtf_failure("Fail: Non-existent ring reported as existing\n");
            return -1;
        }
    }
    else
    {
        if ( !(notify.ents[0].flags & XEN_ARGO_RING_EXISTS) )
        {
            xtf_failure("Fail: Ring not reported as existing\n");
            return -1;
        }
    }

    return 0;
}

/* See the Argo Linux device driver for similar use of these macros */
#define XEN_ARGO_ROUNDUP(x) ROUNDUP(x, XEN_ARGO_MSG_SLOT_SIZE)
#define ARGO_RING_OVERHEAD 80
#define TEST_RING_SIZE(npages)                                      \
    (XEN_ARGO_ROUNDUP((((PAGE_SIZE)*npages) - ARGO_RING_OVERHEAD)))

static int
test_register_ring(domid_t own_domid, xen_argo_port_t aport)
{
    unsigned int i;
    xen_argo_register_ring_t reg = {
        .aport      = aport,
        .partner_id = own_domid,
        .len        = TEST_RING_SIZE(TEST_RING_NPAGES),
    };
    xen_argo_gfn_t gfns[TEST_RING_NPAGES];

    for ( i = 0; i < TEST_RING_NPAGES; i++ )
        gfns[i] = virt_to_gfn(ring_buffer + (i * PAGE_SIZE));

    int rc = hypercall_argo_op(XEN_ARGO_OP_register_ring, &reg, &gfns,
                               TEST_RING_NPAGES, XEN_ARGO_REGISTER_FLAG_FAIL_EXIST);
    switch ( rc )
    {
    case 0:
        return 0;

    case -ENODEV:
        xtf_failure("Fail: Argo is not enabled for this domain\n");
        break;
    case -EFAULT:
        xtf_failure("Fail: Memory fault performing register ring test\n");
        break;
    default:
        xtf_failure("Fail: Unexpected error code %d in register ring test\n", rc);
        break;
    }
    return -1;
}

static int
test_unregister_ring(domid_t partner_domid, xen_argo_port_t aport,
                     bool exists)
{
    xen_argo_register_ring_t unreg = {
        .aport      = aport,
        .partner_id = partner_domid,
    };
    int rc = hypercall_argo_op(XEN_ARGO_OP_unregister_ring, &unreg, NULL, 0, 0);

    switch ( rc )
    {
    case 0:
        if ( exists )
            return 0;
        xtf_failure("Fail: Unexpected success unregistering non-existent ring\n");
        return -1;

    case -ENOENT:
        if ( !exists )
            return 0;
        xtf_failure("Fail: Unexpected ring not found when unregistering \n");
        return -1;

    default:
        xtf_failure("Fail: Unexpected error code %d in unregister ring test\n", rc);
        break;
    }
    return -1;
}

static int
test_sendv(domid_t src_domid, xen_argo_port_t src_aport,
           domid_t dst_domid, xen_argo_port_t dst_aport,
           const char *msg_text, size_t msg_len, unsigned int msg_type)
{
    xen_argo_send_addr_t send_addr = {
        .src = {
            .domain_id = src_domid,
            .aport     = src_aport,
        },
        .dst = {
            .domain_id = dst_domid,
            .aport     = dst_aport,
        },
    };
    xen_argo_iov_t iovs[] = {
        {
            .iov_hnd = _u(msg_text),
            .iov_len = msg_len,
        },
    };
    int rc = hypercall_argo_op(XEN_ARGO_OP_sendv, &send_addr,
                               iovs, ARRAY_SIZE(iovs), msg_type);

    if ( rc < 0 )
    {
        xtf_failure("Fail: Unexpected error code %d in sendv test\n", rc);
        return -1;
    }

    if ( (size_t)rc != msg_len )
    {
        xtf_failure("Fail: Unexpected message size %d written in sendv test\n", rc);
        return -1;
    }

    return 0;
}

static int
inspect_ring_after_first_single_sendv(domid_t src_domid,
                                      xen_argo_port_t src_aport,
                                      const char *sent_msg,
                                      unsigned int sent_msg_len,
                                      unsigned int sent_msg_type)
{
    int rc = 0;
    xen_argo_ring_t *ringp = (xen_argo_ring_t *)ring_buffer;
    struct xen_argo_ring_message_header *msg_hdr;
    unsigned int sent_length;

    if ( ringp->rx_ptr != 0 )
    {
        xtf_failure("Fail: Receive pointer non-zero after sendv: %u\n",
                    ringp->rx_ptr);
        rc = -1;
    }

    if ( ringp->tx_ptr != XEN_ARGO_ROUNDUP(
             sizeof(struct xen_argo_ring_message_header) + sent_msg_len) )
    {
        xtf_failure("Fail: Transmit pointer incorrect after sendv: %u\n",
                    ringp->rx_ptr);
        rc = -1;
    }

    msg_hdr = (struct xen_argo_ring_message_header *)&(ringp->ring);

    if ( msg_hdr->source.domain_id != src_domid )
    {
        xtf_failure("Fail: Source domain id incorrect: %u, expected %u\n",
                    msg_hdr->source.domain_id, src_domid);
        rc = -1;
    }

    if ( msg_hdr->source.aport != src_aport )
    {
        xtf_failure("Fail: Source domain port incorrect: %u, expected %u\n",
                    msg_hdr->source.domain_id, src_aport);
        rc = -1;
    }

    if ( msg_hdr->source.pad != 0 )
    {
        xtf_failure("Fail: Source padding incorrect: %u, expected zero\n",
                    msg_hdr->source.pad);
        rc = -1;
    }

    if ( sent_msg_type != msg_hdr->message_type )
    {
        xtf_failure("Fail: Message type incorrect: %u sent, %u received\n",
                    sent_msg_type, msg_hdr->message_type);
        rc = -1;
    }

    sent_length = sent_msg_len + sizeof(struct xen_argo_ring_message_header);
    if ( sent_length != msg_hdr->len )
    {
        xtf_failure("Fail: Received message length incorrect: "
                    "%u sent is %u with header added, %u received\n",
                    sent_msg_len, sent_length, msg_hdr->len);
        rc = -1;
    }

    if ( memcmp(msg_hdr->data, sent_msg, sent_msg_len) )
    {
        xtf_failure("Fail: Sent message got mangled\n");
        rc = -1;
    }

    return rc;
}

static void clear_test_ring(void)
{
    memset(ring_buffer, 0, sizeof(ring_buffer));
}

void test_main(void)
{
    int own_domid;
    xen_argo_port_t test_aport = 1;
    const char simple_text[] = "A simple thing to send\n";
    const unsigned int msg_type = 0x12345678;

    own_domid = xtf_get_domid();
    if ( own_domid < 0 )
        return xtf_error("Error: could not determine domid of the test domain\n");

    /* First test validates for Argo availability to gate further testing */
    if ( probe_for_argo(own_domid) )
        return;

    if ( test_notify_for_one_ring(own_domid, test_aport, false) ||
         test_unregister_ring(own_domid, test_aport, false) )
        return;

    clear_test_ring();

    if ( test_register_ring(own_domid, test_aport) ||
         test_notify_for_one_ring(own_domid, test_aport, true) ||
         test_unregister_ring(own_domid, test_aport, true) ||
         test_notify_for_one_ring(own_domid, test_aport, false) ||
         test_unregister_ring(own_domid, test_aport, false) )
        return;

    clear_test_ring();

    if ( test_register_ring(own_domid, test_aport) ||
         test_sendv(own_domid, test_aport, own_domid, test_aport,
                    simple_text, strlen(simple_text), msg_type) ||
         inspect_ring_after_first_single_sendv(
             own_domid, test_aport, simple_text, strlen(simple_text), msg_type) ||
         test_notify_for_one_ring(own_domid, test_aport, true) ||
         test_unregister_ring(own_domid, test_aport, true) ||
         test_unregister_ring(own_domid, test_aport, false) )
        return;

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
