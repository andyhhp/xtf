/**
 * @file tests/swint-emulation/lowlevel.h
 *
 * Declarations of stubs and labels in generated in lowlevel.S
 *
 * Nomaclature:
 * - `stub_$X_$Y()`
 *   - Stub function executing instruction `$X` with prefix `$Y`.
 * - `label_$X_$Y_{trap, fault}:`
 *   - Labels for where `$X` is expected to trap or fault.
 *
 * Instructions `$X`:
 * - int3
 *   - Breakpoint (`0xcc`)
 * - int_0x3
 *   - `int $3` (`0xcd 0x03`)
 * - icebp
 *   - Ice Breakpoint (`0xf1`)
 * - int_0x1
 *   - `int $1` (`0xcd 0x01`)
 * - into
 *   - `into` (`0xce`)
 *
 * Prefixes `$Y`:
 * - reg
 *   - Regular - no prefix
 * - red
 *   - Redundant - address size override prefix (`0x67`)
 * - force
 *   - Forced Emulation prefix - see @ref _ASM_XEN_FEP
 * - forcered
 *   - Forced Emulation and redundant prefixes
 */
#ifndef __LOWLEVEL_H__
#define __LOWLEVEL_H__

void stub_int3_reg(void);
void stub_int3_red(void);
void stub_int3_force(void);
void stub_int3_forcered(void);
extern unsigned long label_int3_reg_trap[], label_int3_reg_fault[];
extern unsigned long label_int3_red_trap[], label_int3_red_fault[];
extern unsigned long label_int3_force_trap[], label_int3_force_fault[];
extern unsigned long label_int3_forcered_trap[], label_int3_forcered_fault[];

void stub_int_0x3_reg(void);
void stub_int_0x3_red(void);
void stub_int_0x3_force(void);
void stub_int_0x3_forcered(void);
extern unsigned long label_int_0x3_reg_trap[], label_int_0x3_reg_fault[];
extern unsigned long label_int_0x3_red_trap[], label_int_0x3_red_fault[];
extern unsigned long label_int_0x3_force_trap[], label_int_0x3_force_fault[];
extern unsigned long label_int_0x3_forcered_trap[], label_int_0x3_forcered_fault[];

void stub_icebp_reg(void);
void stub_icebp_red(void);
void stub_icebp_force(void);
void stub_icebp_forcered(void);
extern unsigned long label_icebp_reg_trap[], label_icebp_reg_fault[];
extern unsigned long label_icebp_red_trap[], label_icebp_red_fault[];
extern unsigned long label_icebp_force_trap[], label_icebp_force_fault[];
extern unsigned long label_icebp_forcered_trap[], label_icebp_forcered_fault[];

void stub_int_0x1_reg(void);
void stub_int_0x1_red(void);
void stub_int_0x1_force(void);
void stub_int_0x1_forcered(void);
extern unsigned long label_int_0x1_reg_trap[], label_int_0x1_reg_fault[];
extern unsigned long label_int_0x1_red_trap[], label_int_0x1_red_fault[];
extern unsigned long label_int_0x1_force_trap[], label_int_0x1_force_fault[];
extern unsigned long label_int_0x1_forcered_trap[], label_int_0x1_forcered_fault[];

#ifdef __i386__
void stub_into_reg(void);
void stub_into_red(void);
void stub_into_force(void);
void stub_into_forcered(void);
extern unsigned long label_into_reg_trap[], label_into_reg_fault[];
extern unsigned long label_into_red_trap[], label_into_red_fault[];
extern unsigned long label_into_force_trap[], label_into_force_fault[];
extern unsigned long label_into_forcered_trap[], label_into_forcered_fault[];
#endif

#endif /* __LOWLEVEL_H__ */

/*
 * Local variables:
 * mode: C
 * c-file-style: "BSD"
 * c-basic-offset: 4
 * tab-width: 4
 * indent-tabs-mode: nil
 * End:
 */
