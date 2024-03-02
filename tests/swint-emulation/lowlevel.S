#include <xtf/extable.h>
#include <xtf/asm_macros.h>
#include <xen/arch-x86/xen.h>
#include <arch/processor.h>

        /* Macro to generate a single test function. */
        /* See lowlevel.h for a description of nomenclature. */
.macro GEN_SINGLE name, insn, fep=0, addr=0

ENTRY(stub_\name)

	xor %eax, %eax

        .ifc \insn, into
            /* Ensure the overflow flag is set, to trigger 'into'. */
            mov $0x7f, %dl
            add %dl, %dl
        .endif

        /* Possibly insert a Xen Forced Emulation prefix. */
        .if \fep
            _ASM_XEN_FEP
        .endif

.L_\name\()_fault:

        /*
         * Possibly insert a redundant prefix.
         *
         * Uses 'address size override' which has no effect on any of the
         * target instructions, but in a form which GAS won't complain about.
         */
        .if \addr
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

.L_\name\()_trap:

        ret

	_ASM_EXTABLE_HANDLER(.L_\name\()_fault, .L_\name\()_trap, ex_record_fault_eax)
	_ASM_EXTABLE_HANDLER(.L_\name\()_trap,  .L_\name\()_trap, ex_record_trap_eax)

ENDFUNC(stub_\name\())
.endm

        /* For a single instruction, generate each test variant. */
.macro GEN_SEQUENCE insn
        GEN_SINGLE \insn       \insn
        GEN_SINGLE \insn\()_A  \insn addr=1
        GEN_SINGLE \insn\()_F  \insn fep=1
        GEN_SINGLE \insn\()_FA \insn fep=1 addr=1
.endm

        /* Generate test sequences for each instruction. */
GEN_SEQUENCE int3
GEN_SEQUENCE int_0x3
GEN_SEQUENCE icebp
GEN_SEQUENCE int_0x1
GEN_SEQUENCE into

/*
 * Local variables:
 * tab-width: 8
 * indent-tabs-mode: nil
 * End:
 */
