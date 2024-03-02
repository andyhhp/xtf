/**
 * @file tests/swint-emulation/lowlevel.h
 *
 * Declarations of stubs and labels in generated in lowlevel.S
 *
 * Nomaclature:
 * - `stub_$X_$Y()`
 *   - Stub function executing instruction `$X` with prefix `$Y`.
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
 * - (none)
 *   - Regular - no prefix
 * - A
 *   - Address size override prefix (`0x67`)
 * - F
 *   - Forced Emulation prefix - see @ref _ASM_XEN_FEP
 * - FA
 *   - Forced Emulation and Address override prefixes
 */
#ifndef __LOWLEVEL_H__
#define __LOWLEVEL_H__

unsigned long stub_int3(void);
unsigned long stub_int3_A(void);
unsigned long stub_int3_F(void);
unsigned long stub_int3_FA(void);

unsigned long stub_int_0x3(void);
unsigned long stub_int_0x3_A(void);
unsigned long stub_int_0x3_F(void);
unsigned long stub_int_0x3_FA(void);

unsigned long stub_icebp(void);
unsigned long stub_icebp_A(void);
unsigned long stub_icebp_F(void);
unsigned long stub_icebp_FA(void);

unsigned long stub_int_0x1(void);
unsigned long stub_int_0x1_A(void);
unsigned long stub_int_0x1_F(void);
unsigned long stub_int_0x1_FA(void);

unsigned long stub_into(void);
unsigned long stub_into_A(void);
unsigned long stub_into_F(void);
unsigned long stub_into_FA(void);

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
