/**
 * @file include/arch/x86/config.h
 * Logic to split an environment into finer-grain @#define's
 *
 * Converts `CONFIG_ENV_$foo` into:
 * - `CONFIG_PV` or `CONFIG_HVM`
 * - `CONFIG_PAGING_LEVELS = $num`
 *
 * The `CONFIG_ENV_$foo` is then undefined, to prevent its use in general code.
 */
#ifndef XTF_X86_CONFIG_H
#define XTF_X86_CONFIG_H

#include <xtf/macro_magic.h>

#if defined(CONFIG_ENV_pv64)

#define CONFIG_PV 1
#define CONFIG_64BIT 1
#define CONFIG_PAGING_LEVELS 4
#define ENVIRONMENT_DESCRIPTION "PV 64bit (Long mode 4 levels)"

#undef CONFIG_ENV_pv64

#elif defined(CONFIG_ENV_pv32pae)

#define CONFIG_PV 1
#define CONFIG_32BIT 1
#define CONFIG_PAGING_LEVELS 3
#define ENVIRONMENT_DESCRIPTION "PV 32bit (PAE 3 levels)"

#undef CONFIG_ENV_pv32pae

#elif defined(CONFIG_ENV_hvm64)

#define CONFIG_HVM 1
#define CONFIG_64BIT 1
#define CONFIG_PAGING_LEVELS 4
#define ENVIRONMENT_DESCRIPTION "HVM 64bit (Long mode 4 levels)"

#undef CONFIG_ENV_hvm64

#elif defined(CONFIG_ENV_hvm32pae)

#define CONFIG_HVM 1
#define CONFIG_32BIT 1
#define CONFIG_PAGING_LEVELS 3
#define ENVIRONMENT_DESCRIPTION "HVM 32bit (PAE 3 levels)"

#undef CONFIG_ENV_hvm32pae

#elif defined(CONFIG_ENV_hvm32pse)

#define CONFIG_HVM 1
#define CONFIG_32BIT 1
#define CONFIG_PAGING_LEVELS 2
#define ENVIRONMENT_DESCRIPTION "HVM 32bit (PSE 2 levels)"

#undef CONFIG_ENV_hvm32pse

#elif defined(CONFIG_ENV_hvm32)

#define CONFIG_HVM 1
#define CONFIG_32BIT 1
#define CONFIG_PAGING_LEVELS 0
#define ENVIRONMENT_DESCRIPTION "HVM 32bit (No paging)"

#undef CONFIG_ENV_hvm32

#else
# error Bad environment
#endif

#endif /* XTF_X86_CONFIG_H */

/*
 * Local variables:
 * mode: C
 * c-file-style: "BSD"
 * c-basic-offset: 4
 * tab-width: 4
 * indent-tabs-mode: nil
 * End:
 */
