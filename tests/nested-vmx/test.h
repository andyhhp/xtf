#ifndef VVMX_TEST_H
#define VVMX_TEST_H

#include <xtf.h>

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

extern uint32_t vmcs_revid; /**< Hardware VMCS Revision ID. */

/**
 * Collect real information about the VT-x environment, for use by test.
 */
void vmx_collect_data(void);

/* Clear a VMCS, and set a specific revision id. */
static inline void clear_vmcs(void *_vmcs, uint32_t rev)
{
    uint32_t *vmcs = _vmcs;

    memset(vmcs, 0, PAGE_SIZE);
    vmcs[0] = rev;
}

/* VMX instruction stubs, wrapped to return exinfo_t information. */
exinfo_t stub_vmxon(uint64_t paddr);
exinfo_t stub_vmptrld(uint64_t paddr);
exinfo_t stub_vmxon_user(uint64_t paddr);

/* Test routines. */
void test_msr_vmx(void);
void test_vmxon(void);

#endif /* VVMX_TEST_H */
