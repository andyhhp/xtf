#ifndef XEN_PUBLIC_ARCH_X86_HVM_START_INFO_H
#define XEN_PUBLIC_ARCH_X86_HVM_START_INFO_H

/*
 * Start of day structure passed to PVH guests and to HVM guests in %ebx.
 *
 * NOTE: nothing will be loaded at physical address 0, so a 0 value in any
 * of the address fields should be treated as not present.
 *
 *  0 +----------------+
 *    | magic          | Contains the magic value XEN_HVM_START_MAGIC_VALUE
 *    |                | ("xEn3" with the 0x80 bit of the "E" set).
 *  4 +----------------+
 *    | version        | Version of this structure. Current version is 0. New
 *    |                | versions are guaranteed to be backwards-compatible.
 *  8 +----------------+
 *    | flags          | SIF_xxx flags.
 * 12 +----------------+
 *    | nr_modules     | Number of modules passed to the kernel.
 * 16 +----------------+
 *    | modlist_paddr  | Physical address of an array of modules
 *    |                | (layout of the structure below).
 * 24 +----------------+
 *    | cmdline_paddr  | Physical address of the command line,
 *    |                | a zero-terminated ASCII string.
 * 32 +----------------+
 *    | rsdp_paddr     | Physical address of the RSDP ACPI data structure.
 * 40 +----------------+
 *
 * The layout of each entry in the module structure is the following:
 *
 *  0 +----------------+
 *    | paddr          | Physical address of the module.
 *  8 +----------------+
 *    | size           | Size of the module in bytes.
 * 16 +----------------+
 *    | cmdline_paddr  | Physical address of the command line,
 *    |                | a zero-terminated ASCII string.
 * 24 +----------------+
 *    | reserved       |
 * 32 +----------------+
 *
 * The address and sizes are always a 64bit little endian unsigned integer.
 *
 * NB: Xen on x86 will always try to place all the data below the 4GiB
 * boundary.
 */
#define XEN_HVM_START_MAGIC_VALUE 0x336ec578

struct xen_hvm_start_info {
    uint32_t magic;             /* Contains the magic value 0x336ec578       */
                                /* ("xEn3" with the 0x80 bit of the "E" set).*/
    uint32_t version;           /* Version of this structure.                */
    uint32_t flags;             /* SIF_xxx flags.                            */
    uint32_t nr_modules;        /* Number of modules passed to the kernel.   */
    uint64_t modlist_paddr;     /* Physical address of an array of           */
                                /* hvm_modlist_entry.                        */
    uint64_t cmdline_paddr;     /* Physical address of the command line.     */
    uint64_t rsdp_paddr;        /* Physical address of the RSDP ACPI data    */
                                /* structure.                                */
};
typedef struct xen_hvm_start_info xen_pvh_start_info_t;

struct xen_hvm_modlist_entry {
    uint64_t paddr;             /* Physical address of the module.           */
    uint64_t size;              /* Size of the module in bytes.              */
    uint64_t cmdline_paddr;     /* Physical address of the command line.     */
    uint64_t reserved;
};

#endif /* XEN_PUBLIC_ARCH_X86_HVM_START_INFO_H */
