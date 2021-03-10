/**
 * @file arch/x86/include/arch/asm_macros.h
 *
 * Macros for use in x86 assembly files.
 */
#ifndef XTF_X86_ASM_MACROS_H
#define XTF_X86_ASM_MACROS_H

/* Generate code fragments appropriately for Assembly or C. */
#ifdef __ASSEMBLY__
# define __ASM_CODE(x)     x
# define __ASM_CODE_RAW(x) x
#else
# define __ASM_CODE(x)     " " #x " "
# define __ASM_CODE_RAW(x) #x
#endif

#define ALIGN .align 16

/* Select between two variations based on compat or long mode. */
#ifdef __i386__
# define __ASM_SEL(c, l)     __ASM_CODE(c)
# define __ASM_SEL_RAW(c, l) __ASM_CODE_RAW(c)
#else
# define __ASM_SEL(c, l)     __ASM_CODE(l)
# define __ASM_SEL_RAW(c, l) __ASM_CODE_RAW(l)
#endif

#define _WORD __ASM_SEL(.long, .quad)

#define __ASM_REG(reg) __ASM_SEL_RAW(e ## reg, r ## reg)

#define _ASM_AX __ASM_REG(ax)
#define _ASM_CX __ASM_REG(cx)
#define _ASM_DX __ASM_REG(dx)
#define _ASM_BX __ASM_REG(bx)
#define _ASM_SP __ASM_REG(sp)
#define _ASM_BP __ASM_REG(bp)
#define _ASM_SI __ASM_REG(si)
#define _ASM_DI __ASM_REG(di)

#ifdef __ASSEMBLY__

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

#endif /* __ASSEMBLY__ */

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
