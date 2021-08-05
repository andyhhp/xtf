/*
 * Xen public xen_version hypercall interface
 */
#ifndef __XEN_PUBLIC_VERSION_H__
#define __XEN_PUBLIC_VERSION_H__

/* arg == NULL; returns major:minor (16:16). */
#define XENVER_version      0

/* arg == xen_extraversion_t. (char[16]) */
#define XENVER_extraversion 1
typedef char xen_extraversion_t[16];

/* arg == xen_compile_info_t. */
#define XENVER_compile_info 2
struct xen_compile_info {
    char compiler[64];
    char compile_by[16];
    char compile_domain[32];
    char compile_date[32];
};
typedef struct xen_compile_info xen_compile_info_t;

#define XENVER_changeset 4
typedef char xen_changeset_info_t[64];

#define XENVER_get_features 6
struct xen_feature_info {
    unsigned int submap_idx;    /* IN: which 32-bit submap to return */
    uint32_t     submap;        /* OUT: 32-bit submap */
};
typedef struct xen_feature_info xen_feature_info_t;

/* Declares the features reported by XENVER_get_features. */
#include "features.h"

#endif /* __XEN_PUBLIC_VERSION_H__ */

/*
 * Local variables:
 * mode: C
 * c-file-style: "BSD"
 * c-basic-offset: 4
 * tab-width: 4
 * indent-tabs-mode: nil
 * End:
 */
