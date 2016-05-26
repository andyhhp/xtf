#include <xtf/extable.h>
#include <xtf/asm_macros.h>
#include <xen/arch-x86/xen.h>

#include <arch/idt.h>
#include <arch/msr-index.h>
#include <arch/page.h>
#include <arch/processor.h>
#include <arch/segment.h>

#ifdef __i386__
# define REG  edx
# define REG8 dl
#else
# define REG  rdi
# define REG8 dil
#endif

.macro GEN_SINGLE name, type, force=0

ENTRY(stub_\name) /* exinfo_t stub_$name(unsigned long param) */

        /* Default to no fault.  Filled by ex_check_pf() in exception context. */
        xor %eax, %eax

#ifdef __i386__
        /* Grap 'param' off the stack. */
        mov 4(%esp), %REG
#endif

        /* Insert FEP for the non-exec tests. */
        .ifnc \type, X
            .if \force
                _ASM_XEN_FEP
            .endif
        .endif

        /* Fauling address for the non-exec tests. */
.Lstub_\name\()_fault:

        .ifc \type, R
            /* Read (and trash) the pointer. */
            mov (%REG), %REG8
        .endif

        .ifc \type, I
            /* Load %fs with the selector in %REG.  Causes implicit GDT/LDT access. */
            mov %REG, %fs
        .endif

        .ifc \type, W
            /* Write `ret` to the pointer. */
            movb $0xc3, (%REG)
        .endif

        .ifc \type, X
            /* Call the pointer. */

            .if \force
                /* If force, adjust the function pointer to include the force emulation prefix. */
                sub $5, %REG
            .endif

            call *%REG
        .endif

        /* Fixup address for faults. */
.Lstub_\name\()_fixup:
        ret

ENDFUNC(stub_\name)

        .ifnc \type, X
            _ASM_EXTABLE_HANDLER(.Lstub_\name\()_fault,
                                 .Lstub_\name\()_fixup,
                                 ex_check_pf)
        .endif
.endm

GEN_SINGLE read           R
GEN_SINGLE implicit       I
GEN_SINGLE write          W
GEN_SINGLE exec           X
GEN_SINGLE force_read     R 1
GEN_SINGLE force_implicit I 1
GEN_SINGLE force_write    W 1
GEN_SINGLE force_exec     X 1

.pushsection .text.user, "ax", @progbits

GEN_SINGLE read_user           R
GEN_SINGLE implicit_user       I
GEN_SINGLE write_user          W
GEN_SINGLE exec_user           X
GEN_SINGLE force_read_user     R 1
GEN_SINGLE force_implicit_user I 1
GEN_SINGLE force_write_user    W 1
GEN_SINGLE force_exec_user     X 1

.popsection

#ifdef __x86_64__
ENTRY(stub_64bit_probe_paging_mode)
        push %rbx

        push $(GDTE_CS32_DPL0 * 8)
        push $start_32bit
        lretq

        .code32
start_32bit:

        /* Disable paging. */
        mov %cr0, %esi
        mov %esi, %eax
        and $~X86_CR0_PG, %eax
        mov %eax, %cr0

        /* Flip CR4.PAE and PSE. */
        mov %cr4, %edi
        mov %edi, %eax
        and $~X86_CR4_PAE, %eax
        or  $ X86_CR4_PSE, %eax
        mov %eax, %cr4

        /* Pick %cr3 for 2-level paging. */
        mov $pse_l2_identmap, %eax
        mov %eax, %cr3

        /* Disable long mode. */
        mov $MSR_EFER, %ecx
        rdmsr
        and $~EFER_LME, %eax
        wrmsr

        /* Re-enable paging. */
        mov %esi, %cr0

        /* Query CPUID.1 and stash %edx for the return value. */
        mov $1, %eax
        cpuid
        mov %edx, %ebx

        /* Disable paging. */
        mov %esi, %eax
        and $~X86_CR0_PG, %eax
        mov %eax, %cr0

        /* Enable long mode. */
        mov $MSR_EFER, %ecx
        rdmsr
        or $EFER_LME, %eax
        wrmsr

        /* Restore CR4.PAE. */
        mov %edi, %cr4

        /* Pick %cr3 for 4-level paging. */
        mov $pae_l4_identmap, %eax
        mov %eax, %cr3

        /* Re-enable paging. */
        mov %esi, %cr0

        ljmp $__KERN_CS, $stop_32bit
stop_32bit:
        .code64

        mov %ebx, %eax

        pop %rbx
        ret
ENDFUNC(stub_64bit_probe_paging_mode)
#endif /* __x86_64__ */
