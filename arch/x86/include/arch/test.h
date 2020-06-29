/**
 * @file arch/x86/include/arch/test.h
 *
 * %x86 API for tests.
 */
#ifndef XTF_X86_TEST_H
#define XTF_X86_TEST_H

#include <arch/regs.h>

/**
 * May be implemented by a guest to handle SYSCALL invocations.
 */
void do_syscall(struct cpu_regs *regs);

/**
 * May be implemented by a guest to handle SYSENTER invocations.
 */
void do_sysenter(struct cpu_regs *regs);

/**
 * May be implemented by a guest to handle Event Channel upcalls.
 */
void do_evtchn(struct cpu_regs *regs);

#endif /* XTF_X86_TEST_H */

/*
 * Local variables:
 * mode: C
 * c-file-style: "BSD"
 * c-basic-offset: 4
 * tab-width: 4
 * indent-tabs-mode: nil
 * End:
 */
