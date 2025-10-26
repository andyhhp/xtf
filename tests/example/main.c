/**
 * @file tests/example/main.c
 * @ref test-example - Hello World testcase.
 *
 * @page test-example Example
 *
 * A Hello World XTF testcase.
 *
 * @see tests/example/main.c
 */
#include <xtf.h>

const char test_title[] = "Hello World";

static unsigned long stub_outb(uint8_t port)
{
    unsigned long fault = 0;

    asm ("1: outb %b0, %w[port]; 2:"
         _ASM_EXTABLE_HANDLER(1b, 2b, %P[rec])
         : "+a" (fault) /* Overloaded as the input to OUTB */
         : [rec] "p" (ex_record_fault_eax), [port] "Nd" (port));

    return fault;
}

static unsigned long stub_inb(uint8_t port)
{
    unsigned long fault = 0;

    asm ("1: inb %w[port], %b0; 2:"
         _ASM_EXTABLE_HANDLER(1b, 2b, %P[rec])
         : "+a" (fault)  /* Overloaded as the output of INB */
         : [rec] "p" (ex_record_fault_eax), [port] "Nd" (port) );

    return fault;
}

void test_main(void)
{
    for ( int i = 0; i < 8; ++i )
    {
        exinfo_t fault = stub_outb(i);

        printk("OUTB %u, got %pe\n", i, _p(fault));
    }

    for ( int i = 0; i < 8; ++i )
    {
        exinfo_t fault = stub_inb(i);

        if ( fault & EXINFO_EXPECTED )
            printk(" INB %u, got %pe\n", i, _p(fault));
        else
            printk(" INB %u, got 0x%02x\n", i, fault);

        continue;

        uint8_t val = inb(i);

        if ( val != 0xff )
            printk("Port %03u %#x\n",
                   i, val);
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
