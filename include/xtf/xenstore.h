/**
 * @file include/xtf/xenstore.h
 *
 * Xenstore driver.
 */
#ifndef XTF_XENSTORE_H
#define XTF_XENSTORE_H

/**
 * Issue a #XS_READ operation for @p key, waiting synchronously for the reply.
 *
 * Returns NULL on error.  The current implementation unmarshals data into a
 * static buffer, so the return pointer is only valid until a subsequent
 * xenstore operation.
 */
const char *xenstore_read(const char *key);

#endif /* XTF_XENSTORE_H */

/*
 * Local variables:
 * mode: C
 * c-file-style: "BSD"
 * c-basic-offset: 4
 * tab-width: 4
 * indent-tabs-mode: nil
 * End:
 */
