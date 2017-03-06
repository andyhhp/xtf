#include <xtf/extable.h>
#include <xtf/asm_macros.h>
#include <xen/arch-x86/xen.h>
#include <arch/processor.h>

        /* Macro to generate a single test function. */
        /* See lowlevel.h for a description of nomenclature. */
.macro GEN_SINGLE name, insn, type

        /* Function label.  e.g. stub_int3_red(). */
ENTRY(stub_\name\()_\type)

        .ifc \insn, into
            /* Ensure the overflow flag is set, to trigger 'into'. */
            pushf
            orl $X86_EFLAGS_OF, (%esp)
            popf
        .else
            /* Avoid function label and fault label possibly aliasing. */
            nop
        .endif

        /* Possibly insert a Xen Forced Emulation prefix. */
        .ifc \type, force
            _ASM_XEN_FEP
        .endif
        .ifc \type, forcered
            _ASM_XEN_FEP
        .endif

        /* Label where a fault should occur.  e.g. label_int3_red_fault. */
GLOBAL(label_\name\()_\type\()_fault)

        /*
         * Possibly insert a redundant prefix.
         *
         * Uses 'address size override' which has no effect on any of the
         * target instructions, but in a form which GAS won't complain about.
         */
        .ifc \type, red
            .byte 0x67
        .endif
        .ifc \type, forcered
            .byte 0x67
        .endif

        /* Target instruction to test. */
        .ifc \insn, int3
            int3
        .endif
        .ifc \insn, int_0x3
            .byte 0xcd, 0x03     /* Some assembers turn `int $3` into `int3`. */
        .endif
        .ifc \insn, icebp
            .byte 0xf1
        .endif
        .ifc \insn, int_0x1
            int $0x1
        .endif
        .ifc \insn, into
            .byte 0xce           /* Raw opcode to avoid failure in 64bit build. */
        .endif

        /* Label where a trap should occur.  e.g. label_int3_red_trap. */
GLOBAL(label_\name\()_\type\()_trap)

        /* Function return. */
        ret

        /* Fixup from fault label to trap label. */
        _ASM_EXTABLE(label_\name\()_\type\()_fault,
                     label_\name\()_\type\()_trap)

ENDFUNC(stub_\name\()_\type)
.endm

        /* For a single instruction, generate each test variant. */
.macro GEN_SEQUENCE insn
        GEN_SINGLE \insn \insn reg
        GEN_SINGLE \insn \insn red
        GEN_SINGLE \insn \insn force
        GEN_SINGLE \insn \insn forcered
.endm

        /* Generate test sequences for each instruction. */
GEN_SEQUENCE int3
GEN_SEQUENCE int_0x3
GEN_SEQUENCE icebp
GEN_SEQUENCE int_0x1
GEN_SEQUENCE into

.pushsection .text.user, "ax", @progbits

        /* For a single instruction, generate each user test variant. */
.macro GEN_SEQUENCE_USER insn
        GEN_SINGLE user_\insn \insn reg
        GEN_SINGLE user_\insn \insn red
        GEN_SINGLE user_\insn \insn force
        GEN_SINGLE user_\insn \insn forcered
.endm

        /* Generate user test sequences for each instruction. */
GEN_SEQUENCE_USER int3
GEN_SEQUENCE_USER int_0x3
GEN_SEQUENCE_USER icebp
GEN_SEQUENCE_USER int_0x1
GEN_SEQUENCE_USER into

.popsection

/*
 * Local variables:
 * tab-width: 8
 * indent-tabs-mode: nil
 * End:
 */
