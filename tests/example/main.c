/**
 * @file tests/example/main.c
 * @ref test-example - Hello World testcase.
 *
 * @page test-example Example
 *
 * A Hello World XTF testcase.
 */
#include <xtf/lib.h>

void test_main(void)
{
    printk("Hello World\n");

    xtf_success();
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
