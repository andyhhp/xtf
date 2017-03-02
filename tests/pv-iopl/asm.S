#include <arch/processor.h>
#include <arch/page.h>
#include <arch/segment.h>
#include <xtf/asm_macros.h>

ENTRY(exec_user_with_iopl)      /* void (*fn)(void), unsigned int iopl */
        push %_ASM_BP

        /* Prepare to "call" exec_user_stub(). */
#ifdef __i386__
        mov (1+1)*4(%esp), %eax /* Pass fn() in %eax */
#endif
        push $1f                /* Fake return addr as if we'd called exec_user_stub(). */
        mov %_ASM_SP, %_ASM_BP  /* Stash %esp for entry_ret_to_kernel(). */

        /* Prepare an IRET frame. */
        push $__USER_DS         /* SS */
        push $user_stack + PAGE_SIZE
                                /* ESP */
        pushf                   /* EFLAGS */

                                /* PV guests see the real interrupt flag. Clobber it. */
        andl $~(X86_EFLAGS_IOPL | X86_EFLAGS_IF), (%_ASM_SP)
#ifdef __i386__
        mov  (5+2)*4(%esp), %ecx
        shl  $12, %ecx
        or   %ecx, (%esp)
#else
        shl  $12, %esi
        or   %esi, (%rsp)
#endif

        push $__USER_CS         /* CS */
        push $exec_user_stub    /* EIP */

#ifdef __x86_64__
        push $0
#endif
        jmp HYPERCALL_iret      /* Drop to user privilege. */

1:      /* entry_ret_to_kernel() returns here with a sensible stack. */
        pop %_ASM_BP
        ret

ENDFUNC(exec_user_with_iopl)

/*
 * Local variables:
 * tab-width: 8
 * indent-tabs-mode: nil
 * End:
 */
