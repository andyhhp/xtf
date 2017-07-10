/**
 * @file arch/x86/include/arch/decode.h
 *
 * Helper routines for decoding x86 state.
 */
#ifndef XTF_X86_DECODE_H
#define XTF_X86_DECODE_H

#include <xtf/types.h>

#include <arch/cpuid.h>
#include <arch/exinfo.h>

/**
 * String of the indentified vendor @p v.
 *
 * @param v Vendor.
 * @return String.
 */
const char *x86_vendor_name(enum x86_vendor v);

/**
 * String abbreviation of @p ev.
 *
 * @param ev Entry Vector.
 * @return String abbreviation.
 */
const char *x86_exc_short_name(unsigned int ev);

#endif /* XTF_X86_DECODE_H */

/*
 * Local variables:
 * mode: C
 * c-file-style: "BSD"
 * c-basic-offset: 4
 * tab-width: 4
 * indent-tabs-mode: nil
 * End:
 */
