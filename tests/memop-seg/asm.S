#include <xtf/asm_macros.h>
#include <xtf/extable.h>
#include <arch/segment.h>

        .section .text.user, "ax", @progbits

.macro GEN_ABS_STUB seg load_seg

ENTRY(stub_\seg\()_abs) /* exinfo_t stub_\seg_abs(unsigned long addr) */
#ifdef __i386__
        /*
         * Switch segment if necessary.  The old segment is preserved on the
         * stack for the duration of the test.
         */
        .local stack_adj
        stack_adj = 0

        .if \load_seg
            .ifeqs "\seg", "none"
                push %ds
                push $(GDTE_AVAIL1 << 3 | 3)
                pop  %ds
            .else
                push %\seg
                push $(GDTE_AVAIL1 << 3 | 3)
                pop  %\seg
            .endif
            stack_adj = 1
        .endif
#endif

        /* No exception if we don't fault.  Also reused by the 64bit case. */
        xor %eax, %eax

        /* The bottom bit of 'addr' encodes FEP. */
#ifdef __i386__
        testb $1, (1 + stack_adj)*4(%esp)
#else
        testb $1, %dil
#endif
        jz 1f

        _ASM_XEN_FEP
        .ifeqs "\seg", "none"
#ifdef __i386__
1:          movb $0, 0xc0000000
#else
1:          movabsb %al, 0x8000000040000000
#endif
        .else
#ifdef __i386__
1:          movb $0, %\seg:0xc0000000
#else
1:          movabsb %al, %\seg:0x8000000040000000
#endif
        .endif

2:
#ifdef __i386__
        /* Restore the old segment if necessary. */
        .if \load_seg
            .ifeqs "\seg", "none"
                pop  %ds
            .else
                pop  %\seg
            .endif
        .endif
#endif

        ret

        _ASM_EXTABLE_HANDLER(1b, 2b, ex_record_fault_eax)
ENDFUNC(stub_\seg\()_abs)
.endm

.macro GEN_REG_STUB seg reg load_seg preserve_reg

ENTRY(stub_\seg\()_\reg) /* exinfo_t stub_\seg_\reg(unsigned long addr) */

#ifdef __i386__
        /*
         * Switch segment if necessary.  The old segment is preserved on the
         * stack for the duration of the test.
         */
        .local stack_adj
        stack_adj = 0

        .if \load_seg
            .ifeqs "\seg", "none"
                push %ds
                push $(GDTE_AVAIL1 << 3 | 3)
                pop  %ds
            .else
                push %\seg
                push $(GDTE_AVAIL1 << 3 | 3)
                pop  %\seg
            .endif
            stack_adj = 1
        .endif
#endif

        /* Preserve the subject register if necessary. */
        .if \preserve_reg
            mov %\reg, %_ASM_CX
        .endif

        /* Move 'addr' into \reg */
#ifdef __i386__
        mov (1 + stack_adj)*4(%esp), %\reg
#else
        mov %rdi, %\reg
#endif

        /* The bottom bit of 'addr' encodes FEP. */
        test $1, %\reg
        jz 1f

        _ASM_XEN_FEP
        .ifeqs "\seg", "none"
1:          movb $0, (%\reg)
        .else
1:          movb $0, %\seg:(%\reg)
        .endif

        /* No exception if we didn't fault. */
        xor %eax, %eax
2:
        /* Restore the register if necessary. */
        .if \preserve_reg
            mov %_ASM_CX, %\reg
        .endif

#ifdef __i386__
        /* Restore the old segment if necessary. */
        .if \load_seg
            .ifeqs "\seg", "none"
                pop  %ds
            .else
                pop  %\seg
            .endif
        .endif
#endif

        ret

        _ASM_EXTABLE_HANDLER(1b, 2b, ex_record_fault_eax)
ENDFUNC(stub_\seg\()_\reg)
.endm

/*
 * Instantiate `GEN_ABS_STUB foreach segment` and `GEN_REG_STUB foreach
 * segment, foreach register`, calculaing whether the segment needs loading
 * (implicit %ds, explicit %{e,d,f,g}s), and whether the register needs
 * preserving (depends on the calling ABI).
 */
.local seg_mask, seg_idx, reg_mask, reg_idx
#ifdef __i386__
    seg_mask = 0b1100111
#else
    seg_mask = 0
#endif
    seg_idx  = 6

.irp seg, none, es, cs, ss, ds, fs, gs

    GEN_ABS_STUB \seg, (seg_mask & (1 << seg_idx))

#ifdef __i386__
    reg_mask = 0b00011111
    reg_idx  = 7
    .irp reg, eax, ecx, edx, ebx, esp, ebp, esi, edi
#else
    reg_mask = 0b0001110000001111
    reg_idx  = 15
    .irp reg, rax, rcx, rdx, rbx, rsp, rbp, rsi, rdi, r8, r9, r10, r11, r12, r13, r14, r15
#endif

        GEN_REG_STUB \seg, \reg, (seg_mask & (1 << seg_idx)), (reg_mask & (1 << reg_idx))
        reg_idx = reg_idx - 1

    .endr
    seg_idx = seg_idx - 1

.endr

/*
 * Local variables:
 * tab-width: 8
 * indent-tabs-mode: nil
 * End:
 */
