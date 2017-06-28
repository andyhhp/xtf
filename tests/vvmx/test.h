#ifndef VVMX_TEST_H
#define VVMX_TEST_H

#include <xtf.h>

#include <arch/msr-index.h>
#include <arch/vmx.h>

/*
 * Extentions on top of regular EXINFO.
 *
 * Use EXINFO_AVAIL{0,1} for VMFail{Invalid,Valid}.  (ab)use the fact that
 * VMFailValid means no exception occured to stash the VMX Instruction Error
 * code in the low bits, normally used for vector/error_code information.
 */
#define VMERR_SUCCESS      0
#define VMERR_INVALID      (EXINFO_EXPECTED | EXINFO_AVAIL0)
#define VMERR_VALID(x)     (EXINFO_EXPECTED | EXINFO_AVAIL1 | ((x) & 0xffff))

/**
 * Compare an expectation against what really happenend, printing
 * human-readable information in case of a mismatch.
 */
void check(const char *func, exinfo_t got, exinfo_t exp);

/* Test routines. */
void test_msr_vmx(void);

#endif /* VVMX_TEST_H */
