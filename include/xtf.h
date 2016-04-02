/**
 * @file include/xtf.h
 *
 * Convenience header for tests.
 */
#ifndef XTF_H
#define XTF_H

/* Primitives */
#include <xtf/compiler.h>
#include <xtf/types.h>
#include <xtf/numbers.h>

#include <xtf/lib.h>
#include <xtf/libc.h>

/* Core test functionality */
#include <xtf/report.h>
#include <xtf/test.h>

/* Optional functionality */
#include <xtf/bitops.h>
#include <xtf/exlog.h>
#include <xtf/hypercall.h>
#include <xtf/traps.h>
#include <xtf/xenbus.h>
#include <xtf/xenstore.h>

/* Arch specific headers. */
#include <arch/xtf.h>

#endif /* XTF_H */

/*
 * Local variables:
 * mode: C
 * c-file-style: "BSD"
 * c-basic-offset: 4
 * tab-width: 4
 * indent-tabs-mode: nil
 * End:
 */
