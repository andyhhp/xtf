/**
 * @file include/xtf/grant_table.h
 *
 * A driver for the Xen Grant Table interface.
 */
#ifndef XTF_GRANT_TABLE_H
#define XTF_GRANT_TABLE_H

#include <xtf/hypercall.h>

/**
 * Raw grant table mapping from Xen.
 * Valid once arch_map_gnttab() has returned successfully.
 */
extern uint8_t gnttab_raw[PAGE_SIZE];

/** Grant table in v1 format (aliases #gnttab_raw). */
extern grant_entry_v1_t gnttab_v1[
    sizeof(gnttab_raw) / sizeof(grant_entry_v1_t)];

/** Grant table in v2 format (aliases #gnttab_raw). */
extern grant_entry_v2_t gnttab_v2[
    sizeof(gnttab_raw) / sizeof(grant_entry_v2_t)];

/**
 * Map the domains grant table under #gnttab_raw[].
 */
int arch_map_gnttab(void);


/**
 * Convert a grant status error value to a string.
 * @param err GNTST_*
 * @returns error string, or "unknown"
 */
const char *gntst_strerror(int err);

/**
 * Initialise XTF's grant infrastructure.
 *
 * Sets a grant table version, and maps the grant table itself.  Safe to be
 * called multiple times to switch grant table version, as long as there are
 * no active grants.
 */
int xtf_init_grant_table(unsigned int version);

#endif /* XTF_GRANT_TABLE_H */

/*
 * Local variables:
 * mode: C
 * c-file-style: "BSD"
 * c-basic-offset: 4
 * tab-width: 4
 * indent-tabs-mode: nil
 * End:
 */
