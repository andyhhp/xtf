#include <arch/x86/processor.h>
#include <arch/x86/segment.h>
#include <xtf/asm_macros.h>

ENTRY(exec_user_with_iopl)      /* void (*fn)(void), unsigned int iopl */

        push $__USER_DS         /* SS */
#ifdef __i386__
        push %esp
        addl $4, (%esp)         /* ESP */
#else
        push %rsp
        addq $8, (%rsp)
#endif
        pushf                   /* EFLAGS */

                                /* PV guests see the real interrupt flag. Clobber it. */
#ifdef __i386__
        andl $~(X86_EFLAGS_IOPL | X86_EFLAGS_IF), (%esp)
        mov  5*4(%esp), %eax
        shl  $12, %eax
        or   %eax, (%esp)
#else
        andl $~(X86_EFLAGS_IOPL | X86_EFLAGS_IF), (%rsp)
        shl  $12, %esi
        or   %esi, (%rsp)
#endif

        push $__USER_CS         /* CS */
        push $1f                /* EIP */

#ifdef __x86_64__
        push $0
#endif
        jmp HYPERCALL_iret      /* Drop to user privilege. */
1:
#ifdef __i386__
        call *4(%esp)           /* fn() */
#else
        call *%rdi
#endif

        int $0x20               /* Return to kernel privilege. */
        ret

ENDFUNC(exec_user_with_iopl)

/*
 * Local variables:
 * tab-width: 8
 * indent-tabs-mode: nil
 * End:
 */
