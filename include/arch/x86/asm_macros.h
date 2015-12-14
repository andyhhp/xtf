/**
 * @file include/arch/x86/asm_macros.h
 *
 * Macros for use in x86 assembly files.
 */
#ifndef XTF_X86_ASM_MACROS_H
#define XTF_X86_ASM_MACROS_H

/* Declare data at the architectures width. */
#if defined(__x86_64__)
# define _WORD .quad
#elif defined(__i386__)
# define _WORD .long
#else
# error Bad architecture for _WORD
#endif

.macro SAVE_ALL
    cld
#if defined(__x86_64__)
    push %rdi
    push %rsi
    push %rdx
    push %rcx
    push %rax
    push %r8
    push %r9
    push %r10
    push %r11
    push %rbx
    push %rbp
    push %r12
    push %r13
    push %r14
    push %r15
#elif defined(__i386__)
    push %edi
    push %esi
    push %edx
    push %ecx
    push %eax
    push %ebx
    push %ebp
#else
# error Bad architecture for SAVE_ALL
#endif
.endm

.macro RESTORE_ALL
#if defined(__x86_64__)
    pop %r15
    pop %r14
    pop %r13
    pop %r12
    pop %rbp
    pop %rbx
    pop %r11
    pop %r10
    pop %r9
    pop %r8
    pop %rax
    pop %rcx
    pop %rdx
    pop %rsi
    pop %rdi
#elif defined(__i386__)
    pop %ebp
    pop %ebx
    pop %eax
    pop %ecx
    pop %edx
    pop %esi
    pop %edi
#else
# error Bad architecture for RESTORE_ALL
#endif
.endm

#endif /* XTF_X86_ASM_MACROS_H */

/*
 * Local variables:
 * mode: C
 * c-file-style: "BSD"
 * c-basic-offset: 4
 * tab-width: 4
 * indent-tabs-mode: nil
 * End:
 */
