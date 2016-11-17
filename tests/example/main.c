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

void test_main(void)
{
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
