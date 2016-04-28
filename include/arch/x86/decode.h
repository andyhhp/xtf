/**
 * @file include/arch/x86/decode.h
 *
 * Helper routines for decoding x86 architectural state.
 */
#ifndef XTF_X86_DECODE_H
#define XTF_X86_DECODE_H

#include <xtf/types.h>

/**
 * String abbreviation of @p ev.
 *
 * @param ev Entry Vector.
 * @return String abbreviation.
 */
const char *x86_exc_short_name(unsigned int ev);

/**
 * Decodes an x86 error code into a readable form.
 *
 * @param buf Buffer to fill.
 * @param bufsz Size of @p buf.
 * @param ev Entry Vector.
 * @param ec Error Code.
 * @return snprintf(buf, bufsz, ...)
 */
int x86_exc_decode_ec(char *buf, size_t bufsz,
                      unsigned int ev, unsigned int ec);

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
