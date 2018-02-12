/**
 * @file tests/nested-svm/main.c
 * @ref test-nested-svm
 *
 * @page test-nested-svm Nested SVM
 *
 * Functional testing of the SVM features in a nested-virt environment.
 *
 * @see tests/nested-svm/main.c
 */
#include <xtf.h>

const char test_title[] = "Nested SVM testing";

void test_main(void)
{
    if ( !cpu_has_svm )
        return xtf_skip("Skip: SVM not available\n");

    if ( !vendor_is_amd )
        xtf_warning("Warning: SVM found on non-AMD processor\n");

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
