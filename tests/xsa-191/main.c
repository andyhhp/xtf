/**
 * @file tests/xsa-191/main.c
 * @ref test-xsa-191
 *
 * @page test-xsa-191 XSA-191
 *
 * Advisory: [XSA-191](http://xenbits.xen.org/xsa/advisory-xsa-191.html)
 *
 * Before XSA-191, Xen had several bugs with its handling of segments which
 * shouldn't be eligible for use.  Memory accesses through user segments and
 * loads from the LDT both assumed that the segment was valid for use.
 *
 * The behaviour of loading NULL segments (to invalidate a segment selector)
 * is undocumented, and different across vendors.  Experimentally,
 *
 * - On AMD, the base and limit fields are left stale, and the attributes are
 *   zeroed.
 *
 * - On Intel, the base is zeroed, the limit is set to maximum, and the
 *   attributes are set to a constant value.
 *
 * This test sets up two scenarios which should fail with a @#GP fault, and
 * checks whether a fault is observed.
 *
 * @see tests/xsa-191/main.c
 */
#include <xtf.h>

#include <arch/x86/decode.h>
#include <arch/x86/desc.h>
#include <arch/x86/exinfo.h>
#include <arch/x86/symbolic-const.h>

const char test_title[] = "XSA-191 PoC";

bool test_needs_fep = true;

void test_main(void)
{
    unsigned long tmp;
    exinfo_t fault;

    printk("Testing read through NULL segment:\n");
    write_fs(0);
    asm volatile (_ASM_XEN_FEP
                  "1: mov %%fs:0, %[dst]; 2:"
                  _ASM_EXTABLE_HANDLER(1b, 2b, ex_record_fault_edi)
                  : "=D" (fault),
                    [dst] "=r" (tmp)
                  : "D" (0));

    switch ( fault )
    {
    case 0:
        xtf_failure("  Fail: Access via NULL segment didn't fault\n");
        break;

    case EXINFO_SYM(GP, 0):
        printk("  Success: Got #GP fault\n");
        break;

    default:
    {
        char str[20];

        x86_decode_exinfo(str, ARRAY_SIZE(str), fault);
        xtf_error("  Error: Unexpected fault %#x (%s)\n", fault, str);
        break;
    }
    }

    printk("Testing stale LDT:\n");

    user_desc ldt[1] = { gdt[__KERN_DS >> 3] };

    gdt[GDTE_AVAIL0] =
        (typeof(*gdt))INIT_GDTE((unsigned long)ldt, sizeof(ldt) - 1, 0x82);
    barrier();

    lldt(GDTE_AVAIL0 << 3);
    lldt(0);

    asm volatile (_ASM_XEN_FEP
                  "1: mov %[sel], %%fs; 2:"
                  _ASM_EXTABLE_HANDLER(1b, 2b, ex_record_fault_eax)
                  : "=a" (fault)
                  : "a" (0),
                    [sel] "r" (4));

    switch ( fault )
    {
    case 0:
        xtf_failure("  Fail: Loaded selector from stale LDT\n");
        break;

    case EXINFO_SYM(GP, SEL_EC_SYM(0, LDT)):
        printk("  Success: Got #GP fault\n");
        break;

    default:
    {
        char str[20];

        x86_decode_exinfo(str, ARRAY_SIZE(str), fault);
        xtf_error("  Error: Unexpected fault %#x (%s)\n", fault, str);
        break;
    }
    }

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
